#include <cstdint>

struct __attribute__((packed)) Move {
  uint8_t from_file : 3;
  uint8_t from_rank : 3;
  uint8_t to_file : 3;
  uint8_t to_rank : 3;
  uint8_t promotion : 1;
  uint8_t special : 2; // if promotion = 1
                       // special = 0 == knight promotion
                       // special = 1 == bishop promotion
                       // special = 2 == rook promotion
                       // special = 3 == queen promotion
                       // else
                       // special = 1 == king side castling
                       // special = 2 == queen side castling
                       // special = 3 == en passant
  uint8_t capture : 1;
  uint8_t captured_piece : 6;
};
