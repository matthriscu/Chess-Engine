<<<<<<< HEAD
# This depends on running in the dev branch, with the main branch as a worktree in `main`

current=$(git symbolic-ref --short HEAD)
exe=engine

make

git -C main pull
make -C main

fastchess \
  -engine proto=uci cmd=engine name=dev \
  -engine proto=uci cmd=main/engine name=main \
=======
current=$(git symbolic-ref --short HEAD)
exe=main

git checkout dev
git pull
make
mv "$exe" dev

git checkout main
git pull
make
mv "$exe" main

git checkout "$current"

fastchess \
  -engine proto=uci cmd=dev name=dev \
  -engine proto=uci cmd=main name=main \
>>>>>>> fdf61bc (Update sprt script to compare git branches)
  -each tc=inf/8+0.08 -openings file=books/UHO_Lichess_4852_v1.epd format=epd order=random \
  -games 2 -rounds 2500 -repeat 2 -maxmoves 200 \
  -sprt elo0=0 elo1=10 alpha=0.05 beta=0.1 -concurrency 12 -ratinginterval 10
