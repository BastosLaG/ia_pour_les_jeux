#!/bin/sh

PRG_1=./player_A
PRG_2=./player_B
GAME_DIR=new_stats
NB_GAMES_PER_SIDE=10
NBL=5
NBC=3
CLA_or_MIS_game=0  # CLAssic or MISere
ALT_or_SIM_game=0  # ALTernate or SIMultaneous
CI_or_II_game=0    # Complete Information or Imperfect Information
II_game_type=1     

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

if [ ${ALT_or_SIM_game} == 0 -a ${CI_or_II_game} == 0 ]
then 
  pike run_many_games.pike -f ${PRG_1} -s ${PRG_2} -o ${GAME_DIR} -n ${NB_GAMES_PER_SIDE} -l ${NBL} -c ${NBC} -i 0 -m ${CLA_or_MIS_game} 2>${GAME_DIR}/log1.txt 1>&2
  pike run_many_games.pike -f ${PRG_2} -s ${PRG_1} -o ${GAME_DIR} -n ${NB_GAMES_PER_SIDE} -l ${NBL} -c ${NBC} -i 1 -m ${CLA_or_MIS_game} 2>${GAME_DIR}/log2.txt 1>&2
fi

if [ ${ALT_or_SIM_game} == 0 -a ${CI_or_II_game} == 1 ]
then 
  pike run_many_ii_games.pike -f ${PRG_1} -s ${PRG_2} -o ${GAME_DIR} -n ${NB_GAMES_PER_SIDE} -l ${NBL} -c ${NBC} -i 0 -m ${CLA_or_MIS_game} -d ${II_game_type} 2>${GAME_DIR}/log1.txt 1>&2
  pike run_many_ii_games.pike -f ${PRG_2} -s ${PRG_1} -o ${GAME_DIR} -n ${NB_GAMES_PER_SIDE} -l ${NBL} -c ${NBC} -i 1 -m ${CLA_or_MIS_game} -d ${II_game_type} 2>${GAME_DIR}/log2.txt 1>&2
fi

if [ ${ALT_or_SIM_game} == 1 -a ${CI_or_II_game} == 0 ]
then 
  pike run_many_simultaneous_games.pike -f ${PRG_1} -s ${PRG_2} -o ${GAME_DIR} -n ${NB_GAMES_PER_SIDE} -l ${NBL} -c ${NBC} -i 0 -m ${CLA_or_MIS_game} 2>${GAME_DIR}/log1.txt 1>&2
  pike run_many_simultaneous_games.pike -f ${PRG_2} -s ${PRG_1} -o ${GAME_DIR} -n ${NB_GAMES_PER_SIDE} -l ${NBL} -c ${NBC} -i 1 -m ${CLA_or_MIS_game} 2>${GAME_DIR}/log2.txt 1>&2
fi

echo " ================" > ${GAME_DIR}/resume.txt
echo "  CLA_or_MIS_game ${CLA_or_MIS_game}" >> ${GAME_DIR}/resume.txt
echo "  ALT_or_SIM_game ${ALT_or_SIM_game}" >> ${GAME_DIR}/resume.txt
echo "  CI_or_II_game ${CI_or_II_game}" >> ${GAME_DIR}/resume.txt
if [ ${CI_or_II_game} == 1 ]
then
  echo "    with II_game_type ${II_game_type}" >> ${GAME_DIR}/resume.txt
fi
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

if [ ${ALT_or_SIM_game} == 1 -a ${CI_or_II_game} == 0 ]
then 
  NB_DRAW=$(grep "draw game"  ${GAME_DIR}/scores.txt | wc -l)
  echo " number of draw games "${NB_DRAW} >> ${GAME_DIR}/resume.txt
  NB_NULL=$(grep "null game"  ${GAME_DIR}/scores.txt | wc -l)
  echo " number of null games "${NB_NULL} >> ${GAME_DIR}/resume.txt
  echo " ================" >> ${GAME_DIR}/resume.txt
fi


