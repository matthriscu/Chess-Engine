fastchess \
  -engine proto=uci cmd=$1 name=$1 \
  -engine proto=uci cmd=$2 name=$2 \
  -each tc=inf/8+0.08 -openings file=books/UHO_Lichess_4852_v1.epd format=epd order=random \
  -games 2 -rounds 2500 -repeat 2 -maxmoves 200 \
  -sprt elo0=0 elo1=10 alpha=0.05 beta=0.1 -concurrency 12 -ratinginterval 10
