#!/bin/sh

PRG_1=./rand_player
PRG_2=./ids_player
GAME_DIR=new_stats
NB_GAMES_PER_SIDE=10
NBL=6
NBC=4

if [ -d ${GAME_DIR} ]
then
  echo "GAME_DIR ever here"
  exit 0
else
  mkdir ${GAME_DIR}
fi

if [ ! -e ${PRG_1} ]
then
  echo "PRG_1 is missing"
  exit 0
fi

if [ ! -e ${PRG_2} ]
then
  echo "PRG_2 is missing"
  exit 0
fi

pike run_many_games.pike -f ${PRG_1} -s ${PRG_2} -o ${GAME_DIR} -n ${NB_GAMES_PER_SIDE} -l ${NBL} -c ${NBC} 2>${GAME_DIR}/log1.txt 1>&2
pike run_many_games.pike -f ${PRG_2} -s ${PRG_1} -o ${GAME_DIR} -n ${NB_GAMES_PER_SIDE} -l ${NBL} -c ${NBC} 2>${GAME_DIR}/log2.txt 1>&2

echo " ================" > ${GAME_DIR}/resume.txt
echo "  NB_GAMES_PER_SIDE ${NB_GAMES_PER_SIDE}" >> ${GAME_DIR}/resume.txt
echo "  NBL ${NBL}" >> ${GAME_DIR}/resume.txt
echo "  NBC ${NBC}" >> ${GAME_DIR}/resume.txt
echo "" >> ${GAME_DIR}/resume.txt
echo "  number of wins" >> ${GAME_DIR}/resume.txt
echo " ================" >> ${GAME_DIR}/resume.txt
STR_WIN1=$(echo ${PRG_1} win)
NB_WIN1=$(grep "${STR_WIN1}"  ${GAME_DIR}/scores.txt | wc -l)
echo "    "${PRG_1}" is "${NB_WIN1} >> ${GAME_DIR}/resume.txt

STR_WIN2=$(echo ${PRG_2} win)
NB_WIN2=$(grep "${STR_WIN2}"  ${GAME_DIR}/scores.txt | wc -l)
echo "    "${PRG_2}" is "${NB_WIN2} >> ${GAME_DIR}/resume.txt
echo " ================" >> ${GAME_DIR}/resume.txt
