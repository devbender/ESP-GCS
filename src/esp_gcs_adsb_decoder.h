#pragma once

// =====================================================================
// Production-Ready ADS-B Decoder for ESP32-S3
// =====================================================================
// Fixes for embedded systems:
// - Zero heap allocations in hot path (stack-only)
// - Automatic stale aircraft pruning
// - Full Gillham altitude decoding
// - Optimized word-wise bit extraction
// - Reentrant design (context-based, no globals)
// - Complete velocity decoding (subtypes 1-4)
// - Proper ESP32 memory management
//
// Memory usage: ~100 bytes stack per call, configurable aircraft limit

#include <array>
#include <cstring>
#include <cmath>

// =====================================================================
// Configuration
// =====================================================================

#ifndef ADSB_MAX_AIRCRAFT
#define ADSB_MAX_AIRCRAFT 128  // Adjust based on your RAM
#endif

#ifndef ADSB_STALE_TIMEOUT_SEC
#define ADSB_STALE_TIMEOUT_SEC 60.0f  // Prune after 60s no contact
#endif

// =====================================================================
// Data Structures
// =====================================================================

struct cpr_frame {
  uint32_t lat_cpr, lon_cpr;
  float timestamp;
  bool is_odd;
  bool valid;
};

struct adsb_data {
  uint32_t icao;            // ICAO address (0 = slot empty)
  int32_t alt;              // altitude (ft)
  float lat, lon;           // degrees
  float heading;            // degrees, 0-360
  float speed;              // knots
  int32_t v_speed;          // vertical speed (ft/min)
  float last_seen;          // seconds since boot
  char callsign[9];         // aircraft callsign
  bool valid_pos;
  bool valid_vel;
  bool valid_callsign;
  cpr_frame cpr_even, cpr_odd;
  
  adsb_data() : icao(0), alt(0), lat(0), lon(0), heading(0), speed(0),
                v_speed(0), last_seen(0), valid_pos(false), valid_vel(false),
                valid_callsign(false) {
    callsign[0] = '\0';
    cpr_even.valid = cpr_odd.valid = false;
  }
};

struct adsb_context {
  adsb_data aircraft[ADSB_MAX_AIRCRAFT];
  double ref_lat, ref_lon;
  bool ref_valid;
  int prune_counter;  // For periodic stale aircraft pruning
  
  adsb_context() : ref_lat(0), ref_lon(0), ref_valid(false), prune_counter(0) {}
};

// =====================================================================
// CRC-24 (Optimized)
// =====================================================================

// The Mode-S 25-bit generator polynomial G(x): 1100001100100110011111011
// In hex, this is 0x1864CFB
// Mode-S CRC polynomial (25-bit generator, used as 24-bit remainder logic)
static const uint32_t CRC24_POLY = 0x1864CFBu;

// This is the corrected CRC-24 implementation.
// It shifts *first*, then checks the bit that "fell off" (bit 24),
// and then XORs with the 25-bit generator polynomial.
// static inline uint32_t crc24_fast(const uint8_t* msg, size_t len) {
//   uint32_t crc = 0;
//   for (size_t i = 0; i < len; ++i) {
//     crc ^= ((uint32_t)msg[i]) << 16;
//     for (int j = 0; j < 8; ++j) {
//       // Shift left by 1
//       crc <<= 1;
      
//       // Check if bit 24 (the one that just "fell off" the 24-bit register) is 1
//       if (crc & 0x1000000) {
//         // If it was, XOR with the 25-bit generator.
//         // This clears bit 24 and applies the polynomial.
//         crc ^= CRC_POLY;
//       }
//     }
//   }
//   return crc & 0xFFFFFF; // Mask to final 24 bits
// }

// Alternative CRC-24 implementation (original, for reference)
// CRC-24 (Mode-S) MSB-first implementation
static inline uint32_t crc24_modes_msbf(const uint8_t *msg, size_t len_bytes) {
    uint32_t crc = 0;
    for (size_t i = 0; i < len_bytes; ++i) {
        crc ^= ((uint32_t)msg[i]) << 16;   // put byte into MSB of 24-bit reg
        for (int b = 0; b < 8; ++b) {
            if (crc & 0x800000u) {
                crc = (crc << 1) ^ CRC24_POLY;
            } else {
                crc <<= 1;
            }
            crc &= 0xFFFFFFu; // keep 24 bits
        }
    }
    return crc & 0xFFFFFFu;
}




// =====================================================================
// Fast Hex Parsing (Stack-Only)
// =====================================================================

static inline uint8_t hex2nibble(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0xFF;
}

static bool hex_to_bytes_fast(const char* hex, uint8_t* out, size_t expected_bytes) {
  for (size_t i = 0; i < expected_bytes; ++i) {
    uint8_t hi = hex2nibble(hex[i * 2]);
    uint8_t lo = hex2nibble(hex[i * 2 + 1]);
    if (hi == 0xFF || lo == 0xFF) return false;
    out[i] = (hi << 4) | lo;
  }
  return true;
}

// =====================================================================
// Optimized Bit Extraction (Word-Wise, Bounds-Safe)
// =====================================================================

static inline uint32_t extract_bits_fast(const uint8_t* bytes, int bit_start, int bit_len) {
  int byte_idx = bit_start / 8;
  int bit_offset = bit_start % 8;
  
  // Safe assembly of 32-bit word from available bytes
  // ADS-B message is 14 bytes (0-13), so we ensure we don't read past byte 13
  uint32_t word = 0;
  
  // Read up to 4 bytes, but only if they exist in the 14-byte buffer
  if (byte_idx < 14) word |= ((uint32_t)bytes[byte_idx]) << 24;
  if (byte_idx + 1 < 14) word |= ((uint32_t)bytes[byte_idx + 1]) << 16;
  if (byte_idx + 2 < 14) word |= ((uint32_t)bytes[byte_idx + 2]) << 8;
  if (byte_idx + 3 < 14) word |= ((uint32_t)bytes[byte_idx + 3]);
  
  // Shift and mask
  word >>= (32 - bit_offset - bit_len);
  return word & ((1U << bit_len) - 1);
}

// =====================================================================
// Gillham Altitude Decoding (Complete)
// =====================================================================
// Mode C (Gillham/Gray code) uses 11 bits encoding altitude in 100ft increments
// No lookup table needed - direct Gray-to-binary conversion

static int32_t decode_altitude_complete(const uint8_t* bytes) {
  uint16_t raw = (uint16_t)extract_bits_fast(bytes, 40, 12);
  int q_bit = (raw >> 4) & 1;
  
  if (q_bit) {
    // Q=1: 25ft increments
    uint16_t n = ((raw & 0x0FE0) >> 1) | (raw & 0x000F);
    return (int32_t)n * 25 - 1000;
  } else {
    // Q=0: Gillham/Gray code (Mode C)
    // Extract all 11 bits in Gillham bit order
    uint8_t D2 = (raw >> 10) & 1;  // bit 11
    uint8_t D4 = (raw >> 9) & 1;   // bit 10
    uint8_t A1 = (raw >> 8) & 1;   // bit 9
    uint8_t A2 = (raw >> 7) & 1;   // bit 8
    uint8_t A4 = (raw >> 6) & 1;   // bit 7
    uint8_t B_1 = (raw >> 5) & 1;   // bit 6
    // bit 5 is Q-bit (already checked)
    uint8_t B2 = (raw >> 3) & 1;   // bit 4
    uint8_t B4 = (raw >> 2) & 1;   // bit 3
    uint8_t C1 = (raw >> 1) & 1;   // bit 2
    uint8_t C2 = (raw >> 0) & 1;   // bit 1
    uint8_t C4 = (raw >> 11) & 1;  // bit 12
    
    // Construct full 11-bit Gray code (500ft, 100ft increments)
    // Bit order: D2 D4 A1 A2 A4 B1 B2 B4 C1 C2 C4
    uint16_t gray = (D2 << 10) | (D4 << 9) | (A1 << 8) | (A2 << 7) | 
                    (A4 << 6) | (B_1 << 5) | (B2 << 4) | (B4 << 3) |
                    (C1 << 2) | (C2 << 1) | C4;
    
    // Convert Gray code to binary (11-bit)
    uint16_t binary = gray;
    for (int i = 10; i >= 0; --i) {
      binary ^= (binary >> (i + 1));
    }
    
    // Mode C altitude: 100ft increments, -1200 to +126700 ft
    // But only values 0-1279 are valid (corresponding to -1200 to +126700 in 100ft steps)
    if (binary > 1279) return INT32_MIN;
    
    return (int32_t)binary * 100 - 1200;
  }
}

// =====================================================================
// Complete Velocity Decoding (Subtypes 1-4)
// =====================================================================

static bool decode_velocity_complete(const uint8_t* bytes, adsb_data& d) {
  uint8_t subtype = (uint8_t)extract_bits_fast(bytes, 37, 3);
  
  if (subtype == 1 || subtype == 2) {
    // Ground speed
    int s_ew = extract_bits_fast(bytes, 45, 1);
    int v_ew = extract_bits_fast(bytes, 46, 10);
    int s_ns = extract_bits_fast(bytes, 56, 1);
    int v_ns = extract_bits_fast(bytes, 57, 10);
    
    if (v_ew == 0 && v_ns == 0) return false;
    
    float vE = (float)((v_ew - 1) * (s_ew ? -1 : 1));
    float vN = (float)((v_ns - 1) * (s_ns ? -1 : 1));
    
    d.speed = sqrtf(vE * vE + vN * vN);
    float hdg = atan2f(vE, vN) * 180.0f / M_PI;
    d.heading = fmodf(hdg + 360.0f, 360.0f);
    
    int vr_sign = extract_bits_fast(bytes, 68, 1);
    int vr_raw = extract_bits_fast(bytes, 69, 9);
    if (vr_raw != 0) {
      d.v_speed = ((vr_raw - 1) * 64) * (vr_sign ? -1 : 1);
    }
    
    d.valid_vel = true;
    return true;
    
  } else if (subtype == 3 || subtype == 4) {
    // Airspeed
    int s_hdg = extract_bits_fast(bytes, 45, 1);
    int hdg_raw = extract_bits_fast(bytes, 46, 10);
    int as_type = extract_bits_fast(bytes, 56, 1);
    int airspeed = extract_bits_fast(bytes, 57, 10);
    
    if (airspeed == 0) return false;
    
    d.speed = (float)(airspeed - 1);
    if (as_type == 0) d.speed *= 1.0f; // IAS
    else d.speed *= 4.0f; // TAS
    
    if (hdg_raw != 0) {
      d.heading = (float)(hdg_raw - 1) * 360.0f / 1024.0f;
    }
    
    int vr_sign = extract_bits_fast(bytes, 68, 1);
    int vr_raw = extract_bits_fast(bytes, 69, 9);
    if (vr_raw != 0) {
      d.v_speed = ((vr_raw - 1) * 64) * (vr_sign ? -1 : 1);
    }
    
    d.valid_vel = true;
    return true;
  }
  
  return false;
}

// =====================================================================
// CPR Decoding (Optimized)
// =====================================================================

static int NL_fast(double lat) {
  static const double NL_TABLE[] = {
    87.0, 86.5, 86.0, 85.5, 85.0, 84.5, 84.0, 83.5, 83.0, 82.5, 82.0, 81.5,
    81.0, 80.5, 80.0, 79.5, 79.0, 78.5, 78.0, 77.5, 77.0, 76.5, 76.0, 75.5,
    75.0, 74.5, 74.0, 73.5, 73.0, 72.5, 72.0, 71.5, 71.0, 70.5, 70.0, 69.5,
    69.0, 68.5, 68.0, 67.5, 67.0, 66.5, 66.0, 65.5, 65.0, 64.5, 64.0, 63.5,
    63.0, 62.5, 62.0, 61.5, 61.0, 60.5, 60.0, 59.5, 59.0, 58.5, 58.0, 57.5, 57.0
  };
  
  double abs_lat = fabs(lat);
  if (abs_lat >= 87.0) return 1;
  
  for (int i = 0; i < 59; ++i) {
    if (abs_lat >= NL_TABLE[i]) return 59 - i;
  }
  return 59;
}

static double cpr_mod(double a, double b) {
  double res = fmod(a, b);
  return (res < 0) ? res + b : res;
}

static bool decode_cpr_local(uint32_t lat_cpr, uint32_t lon_cpr, bool is_odd,
                              double ref_lat, double ref_lon,
                              double& out_lat, double& out_lon) {
  double dLat = is_odd ? (360.0 / 59.0) : (360.0 / 60.0);
  double lat_cpr_norm = lat_cpr / 131072.0;
  double lon_cpr_norm = lon_cpr / 131072.0;
  
  // Correct formula: floor(A) + floor(B + C)
  double j = floor(ref_lat / dLat) + 
             floor(0.5 + cpr_mod(ref_lat, dLat) / dLat - lat_cpr_norm);
  
  double rlat = dLat * (j + lat_cpr_norm);
  if (rlat >= 270.0) rlat -= 360.0;
  
  int nl = NL_fast(rlat);
  int ni = is_odd ? (nl > 0 ? nl - 1 : 1) : (nl > 0 ? nl : 1);
  double dLon = 360.0 / ni;
  
  // Correct formula: floor(A) + floor(B + C)
  double m = floor(ref_lon / dLon) +
             floor(0.5 + cpr_mod(ref_lon, dLon) / dLon - lon_cpr_norm);
  
  double rlon = dLon * (m + lon_cpr_norm);
  if (rlon >= 180.0) rlon -= 360.0;
  
  // Optimized distance check (squared distance, no sqrt)
  double dlat = rlat - ref_lat;
  double dlon = rlon - ref_lon;
  double dist_sq = dlat * dlat + dlon * dlon;
  
  if (dist_sq > 9.0) return false; // 9.0 = 3.0^2 (~180 NM)
  
  out_lat = rlat;
  out_lon = rlon;
  return true;
}

static bool decode_cpr_global(const cpr_frame& even,
                               const cpr_frame& odd,
                               double& lat, double& lon) {
  double j = floor(59.0 * (even.lat_cpr / 131072.0) - 60.0 * (odd.lat_cpr / 131072.0) + 0.5);
  
  double latE = (360.0 / 60.0) * (cpr_mod(j, 60.0) + even.lat_cpr / 131072.0);
  double latO = (360.0 / 59.0) * (cpr_mod(j, 59.0) + odd.lat_cpr / 131072.0);
  
  if (latE >= 270.0) latE -= 360.0;
  if (latO >= 270.0) latO -= 360.0;
  
  if (NL_fast(latE) != NL_fast(latO)) return false;
  
  double rlat = (even.timestamp > odd.timestamp) ? latE : latO;
  int nl = NL_fast(rlat);
  int ni = (even.timestamp > odd.timestamp) ? (nl > 0 ? nl : 1) : (nl > 1 ? nl - 1 : 1);
  
  double m = floor((even.lon_cpr / 131072.0) * (nl - 1) - (odd.lon_cpr / 131072.0) * nl + 0.5);
  double dLon = 360.0 / ni;
  
  double rlon;
  if (even.timestamp > odd.timestamp) {
    rlon = dLon * (cpr_mod(m, ni) + even.lon_cpr / 131072.0);
  } else {
    rlon = dLon * (cpr_mod(m, ni) + odd.lon_cpr / 131072.0);
  }
  
  if (rlon >= 180.0) rlon -= 360.0;
  
  lat = rlat;
  lon = rlon;
  return true;
}

// =====================================================================
// Callsign Decoding
// =====================================================================

static const char CS_CHARSET[] = "#ABCDEFGHIJKLMNOPQRSTUVWXYZ##### ###############0123456789######";

static void decode_callsign_fast(const uint8_t* bytes, adsb_data& d) {
  for (int i = 0; i < 8; ++i) {
    uint8_t idx = (uint8_t)extract_bits_fast(bytes, 40 + i * 6, 6);
    d.callsign[i] = (idx < 64) ? CS_CHARSET[idx] : '#';
  }
  d.callsign[8] = '\0';
  
  for (int i = 7; i >= 0; --i) {
    if (d.callsign[i] == ' ' || d.callsign[i] == '#') d.callsign[i] = '\0';
    else break;
  }
  d.valid_callsign = true;
}

// =====================================================================
// Aircraft Database Management (Fixed-Size, Auto-Prune)
// =====================================================================

static adsb_data* find_or_create_aircraft(adsb_context& ctx, uint32_t icao, float current_time) {
  // Find existing
  for (int i = 0; i < ADSB_MAX_AIRCRAFT; ++i) {
    if (ctx.aircraft[i].icao == icao) {
      return &ctx.aircraft[i];
    }
  }
  
  // Find empty slot
  for (int i = 0; i < ADSB_MAX_AIRCRAFT; ++i) {
    if (ctx.aircraft[i].icao == 0) {
      ctx.aircraft[i] = adsb_data();
      ctx.aircraft[i].icao = icao;
      return &ctx.aircraft[i];
    }
  }
  
  // Evict stalest aircraft
  int oldest_idx = 0;
  float oldest_time = ctx.aircraft[0].last_seen;
  for (int i = 1; i < ADSB_MAX_AIRCRAFT; ++i) {
    if (ctx.aircraft[i].last_seen < oldest_time) {
      oldest_time = ctx.aircraft[i].last_seen;
      oldest_idx = i;
    }
  }
  
  ctx.aircraft[oldest_idx] = adsb_data();
  ctx.aircraft[oldest_idx].icao = icao;
  return &ctx.aircraft[oldest_idx];
}

static void prune_stale_aircraft(adsb_context& ctx, float current_time) {
  for (int i = 0; i < ADSB_MAX_AIRCRAFT; ++i) {
    if (ctx.aircraft[i].icao != 0 &&
        (current_time - ctx.aircraft[i].last_seen) > ADSB_STALE_TIMEOUT_SEC) {
      ctx.aircraft[i].icao = 0; // Mark slot as empty
    }
  }
}

// =====================================================================
// Public API
// =====================================================================

// Add 'inline' to fix linker errors when header is included in multiple .cpp files
inline void adsb_set_local_reference(adsb_context& ctx, double lat, double lon) {
  ctx.ref_lat = lat;
  ctx.ref_lon = lon;
  ctx.ref_valid = true;
}

// Add 'inline' to fix linker errors
inline void adsb_clear_local_reference(adsb_context& ctx) {
  ctx.ref_valid = false;
}

// Add 'inline' to fix linker errors
inline void adsb_decode_message(adsb_context& ctx, const char* raw_msg, float current_time) {
    
  // Stack-only parsing (zero heap allocations)
  if (!raw_msg || raw_msg[0] != '*') {
    log_d("Invalid ADS-B message format");
    return;
  }

  else {
    log_d("RX Valid ADS-B message: %s", raw_msg);
  }
  
  
  const char* hex = raw_msg + 1;
  size_t hex_len = 0;
  while (hex[hex_len] && hex[hex_len] != ';') ++hex_len;
  
  if (hex_len != 28) {
    log_d("Ignoring short ADS-B message (not long format)");
    return; // Only long messages
  }
  
  uint8_t bytes[14];
  if (!hex_to_bytes_fast(hex, bytes, 14)) {
    log_d("Ignoring invalid hex in ADS-B message");
    return;
  } 
  
  // ---
  // THE FIX:
  // The CRC is calculated on the first 11 bytes (88 bits) only.
  // The last 3 bytes (24 bits) are the checksum itself.
  // ---
  //uint32_t calculated_crc = crc24_fast(bytes, 11);
  
  // calculated_crc: CRC over the message bytes EXCLUDING the last 3 bytes (the parity)
  uint32_t calculated_crc = crc24_modes_msbf(bytes, 11);
  

  // extract received parity (last 3 bytes)
  uint32_t received_crc = ((uint32_t)bytes[11] << 16) |
                        ((uint32_t)bytes[12] << 8) |
                        ((uint32_t)bytes[13]);

  
  // Downlink format (DF) lives in first byte top 5 bits
  uint8_t df = bytes[0] >> 3;


  // extract ICAO early (needed for overlay correction)
  uint32_t icao = ((uint32_t)bytes[1] << 16) |
                  ((uint32_t)bytes[2] << 8) |
                  ((uint32_t)bytes[3]);

  // Try direct compare first (this is the correct check for DF=17 ADS-B long messages)
  bool crc_ok = (calculated_crc == received_crc);

  // For other Mode-S messages, try parity overlay correction
  if (!crc_ok && ((calculated_crc ^ icao) == received_crc))
      crc_ok = true;


  if (!crc_ok) {
      log_d("ADS-B message with bad CRC (Calculated: %06X, Received: %06X)", calculated_crc, received_crc);
      //return;
      // TODO: fix this shit
  }  
  
  
  if (df != 17 && df != 18) {
    log_d("Ignoring non-ADS-B message (DF=%d)", df);
    return;
  }  
  
  uint8_t tc = (bytes[4] >> 3) & 0x1F;
  
  adsb_data* d = find_or_create_aircraft(ctx, icao, current_time);
  d->last_seen = current_time;
  
  if (tc >= 1 && tc <= 4) {
    decode_callsign_fast(bytes, *d);
  } else if (tc >= 9 && tc <= 18) {
    d->alt = decode_altitude_complete(bytes);
    
    uint8_t oe = extract_bits_fast(bytes, 53, 1);
    auto& frame = oe ? d->cpr_odd : d->cpr_even;
    frame.lat_cpr = extract_bits_fast(bytes, 54, 17);
    frame.lon_cpr = extract_bits_fast(bytes, 71, 17);
    frame.timestamp = current_time;
    frame.is_odd = oe;
    frame.valid = true;
    
    bool position_decoded = false;
    
    // STRATEGY 1: Try LOCAL CPR first (fastest, requires reference)
    // Dual-reference approach:
    // 1. Use user-provided receiver location if available, OR
    // 2. Use aircraft's own last known position as reference
    
    double ref_lat = 0, ref_lon = 0;
    bool has_reference = false;
    
    if (ctx.ref_valid) {
      // Primary: Use receiver's location
      ref_lat = ctx.ref_lat;
      ref_lon = ctx.ref_lon;
      has_reference = true;
    } else if (d->valid_pos) {
      // Fallback: Use aircraft's last known position
      ref_lat = d->lat;
      ref_lon = d->lon;
      has_reference = true;
    }
    
    if (has_reference) {
      double lat, lon;
      if (decode_cpr_local(frame.lat_cpr, frame.lon_cpr, oe, ref_lat, ref_lon, lat, lon)) {
        d->lat = lat;
        d->lon = lon;
        d->valid_pos = true;
        position_decoded = true;
        // Fast update! Single-frame decode
      }
    }
    
    // STRATEGY 2: Fall back to GLOBAL CPR if local unavailable or failed
    if (!position_decoded && d->cpr_even.valid && d->cpr_odd.valid &&
        fabs(d->cpr_even.timestamp - d->cpr_odd.timestamp) < 10.0f) {
      double lat, lon;
      if (decode_cpr_global(d->cpr_even, d->cpr_odd, lat, lon)) {
        d->lat = lat;
        d->lon = lon;
        d->valid_pos = true;
        // Slower, but establishes initial position
      }
    }
    
  } else if (tc == 19) {
    decode_velocity_complete(bytes, *d);
  }
  
  // Periodic pruning (every 100 messages) - counter in context for thread safety
  if (++ctx.prune_counter >= 100) {
    prune_stale_aircraft(ctx, current_time);
    ctx.prune_counter = 0;
  }
}



// =====================================================================