#!/usr/bin/env bash

echo -e "To remove colour from tests, set COLOUR to 1 in sh file\n"
COLOUR=0
if [[ COLOUR -eq 0 ]]; then
        ORANGE='\033[0;33m'
        GREEN='\033[0;32m'
        RED='\033[0;31m'
        NC='\033[0m'
else
        ORANGE='\033[0m'
        GREEN='\033[0m'
        RED='\033[0m'
        NC='\033[0m'
fi

SCORE=0

dir=$(pwd)

cd $dir
echo -e "\nTesting :: Compilation\n"
make clean >/dev/null 2>&1
make >/dev/null 2>&1
if [ $? -eq 0 ]; then
        echo -e "  ${GREEN}Compilation Test Passed${NC}"
        SCORE=$(bc -l <<< "$SCORE+5")
else
        echo -e "  ${RED}Failed${NC}"
        echo $SCORE/100
        exit 0 
fi

cd $dir
echo -e "\nTesting :: Memory Leaks\n"
./shell < ./test-files/leaks.txt >/dev/null 2>&1
if [ $? -eq 0 ]; then
        echo -e "  ${GREEN}Leaks Test Passed${NC}"
        SCORE=$(bc -l <<< "$SCORE+10")
else
        echo -e "  ${RED}Failed${NC}"
fi
rm -rf a b output.txt

cd $dir
echo -e "\nTesting :: Username\n"
cat ./test-files/test_exit.txt > ./test-files/cmd.txt
./shell < ./test-files/cmd.txt >temp 2>/dev/null
if grep -qF -- "${USER}" temp; then
        echo -e "  ${GREEN}Test One Passed${NC}"
        SCORE=$(bc -l <<< "$SCORE+5")
else
        echo -e "  ${RED}Failed${NC}"
fi
rm temp

cd $dir
echo -e "\nTesting :: Current Working Directory\n"
cat ./test-files/test_exit.txt > ./test-files/cmd.txt
./shell < ./test-files/cmd.txt >temp 2>/dev/null
if grep -qF -- "$(pwd)" temp; then
        echo -e "  ${GREEN}Test Two Passed${NC}"
        SCORE=$(bc -l <<< "$SCORE+5")
else
        echo -e "  ${RED}Failed${NC}"
fi
rm temp

cd $dir
echo -e "\nTesting :: echo \"Hello world | Life is Good > Great $\"\n"
cat ./test-files/test_echo_double.txt ./test-files/test_exit.txt > ./test-files/cmd.txt
RES=$(. ./test-files/test_echo_double.txt)
if ./shell < ./test-files/cmd.txt 2>/dev/null | grep -qF -- "${RES}"; then
        echo -e "  ${GREEN}Test Three Passed${NC}"
        SCORE=$(bc -l <<< "$SCORE+5")
else
        echo -e "  ${RED}Failed${NC}"
fi

cd $dir
echo -e "\nTesting :: ls\n"
cat ./test-files/test_ls.txt ./test-files/test_exit.txt > ./test-files/cmd.txt
RES=$(. ./test-files/test_ls.txt)
if ./shell < ./test-files/cmd.txt 2>/dev/null | grep -qF -- "${RES}"; then
        echo -e "  ${GREEN}Test Four Passed${NC}"
        SCORE=$(bc -l <<< "$SCORE+2.5")
else
        echo -e "  ${RED}Failed${NC}"
fi

cd $dir
echo -e "\nTesting :: ls -l n"
cat ./test-files/test_ls_l.txt ./test-files/test_exit.txt > ./test-files/cmd.txt
RES=$(. ./test-files/test_ls_l.txt)
if ./shell < ./test-files/cmd.txt 2>/dev/null | grep -qF -- "${RES}"; then
        echo -e "  ${GREEN}Test Five Passed${NC}"
        SCORE=$(bc -l <<< "$SCORE+2.5")
else
        echo -e "  ${RED}Failed${NC}"
fi

cd $dir
echo -e "\nTesting :: ls -l -a\n"
cat ./test-files/test_ls_l_a.txt ./test-files/test_exit.txt > ./test-files/cmd.txt
RES=$(. ./test-files/test_ls_l_a.txt)
if ./shell < ./test-files/cmd.txt 2>/dev/null | grep -qF -- "${RES}"; then
        echo -e "  ${GREEN}Test Six Passed${NC}"
        SCORE=$(bc -l <<< "$SCORE+2.5")
else
        echo -e "  ${RED}Failed${NC}"
fi

cd $dir
echo -e "\nTesting :: ps aux\n"
cat ./test-files/test_ps_aux.txt ./test-files/test_exit.txt > ./test-files/cmd.txt
RES=$(. ./test-files/test_ps_aux.txt)
if ./shell < ./test-files/cmd.txt 2>/dev/null | grep -qF -- "${RES}"; then
        echo -e "  ${GREEN}Test Seven Passed${NC}"
        SCORE=$(bc -l <<< "$SCORE+2.5")
else
        echo -e "  ${RED}Failed${NC}"
fi

cd $dir
echo -e "\nTesting :: ps aux > a; grep /init < a; grep /init < a > b\n"
cat ./test-files/test_input_output_redirection.txt ./test-files/test_exit.txt > ./test-files/cmd.txt
RES=$(. ./test-files/test_input_output_redirection.txt)
rm -f a b
./shell < ./test-files/cmd.txt >temp 2>/dev/null
if grep -qF -- "${RES}" temp; then
        if [ -f a ] && [ -f b ] && grep -qF -- "${RES}" b; then
                echo -e "  ${GREEN}Test Eight Passed${NC}"
                SCORE=$(bc -l <<< "$SCORE+15")
        else
                echo -e "  ${RED}Failed file creation${NC}"
        fi
else
        echo -e "  ${RED}Failed final output${NC}"
fi
rm temp

cd $dir
echo -e "\nTesting :: ls -l | grep \"shell.cpp\"\n"
cat ./test-files/test_single_pipe.txt ./test-files/test_exit.txt > ./test-files/cmd.txt
RES=$(. ./test-files/test_single_pipe.txt)
NOTRES=$(ls -l | grep "Tokenizer.cpp")
strace -e trace=execve -f -o out.trace ./shell < ./test-files/cmd.txt >temp 2>/dev/null
LS=$(which ls)
GREP=$(which grep)
if grep -q "execve(\"${LS}\"" out.trace && grep -q "execve(\"${GREP}\"" out.trace && grep -qF -- "${RES}" temp && ! grep -qFw -- "${NOTRES}" temp; then
        echo -e "  ${GREEN}Test Nine Passed${NC}"
        SCORE=$(bc -l <<< "$SCORE+8")
else
        echo -e "  ${RED}Failed${NC}"
fi
rm temp

cd $dir
echo -e "\nTesting :: ps aux | awk ""'""/usr/{print \$1}""'"" | sort -r\n"
cat ./test-files/test_multiple_pipes_A.txt ./test-files/test_exit.txt > ./test-files/cmd.txt
RES=$(. ./test-files/test_multiple_pipes_A.txt)
ARR=($RES)
echo "${RES}" >cnt.txt
CNT=$(grep -oF -- "${ARR[0]}" cnt.txt | wc -l)
strace -e trace=execve -f -o out.trace ./shell < ./test-files/cmd.txt >temp 2>/dev/null
PS=$(which ps)
AWK=$(which awk)
SORT=$(which sort)
if grep -q "execve(\"${PS}\"" out.trace && grep -q "execve(\"${AWK}\"" out.trace && grep -q "execve(\"${SORT}\"" out.trace && grep -qF -- "${RES}" temp && [ $(grep -oFw -- "${ARR[0]}" temp | wc -l) -le $((${CNT}+3)) ]; then
        echo -e "  ${GREEN}Test Ten Passed${NC}"
        SCORE=$(bc -l <<< "$SCORE+6")
else
        echo -e "  ${RED}Failed${NC}"
fi
rm cnt.txt temp

cd $dir
echo -e "\nTesting :: Multiple Pipes & Redirection\n"
cat ./test-files/test_multiple_pipes_redirection.txt ./test-files/test_exit.txt > ./test-files/cmd.txt
RES=$(. ./test-files/test_multiple_pipes_redirection.txt)
echo "${RES}" >cnt.txt
CNT=$(grep -oF -- "${RES}" cnt.txt | wc -l)
rm -f cnt.txt test.txt output.txt
if [ $(./shell < ./test-files/cmd.txt 2>/dev/null | grep -oF -- "${RES}" | wc -l) -eq ${CNT} ] && [ -f test.txt ] && [ -f output.txt ]; then
        echo -e "  ${GREEN}Test Eleven Passed${NC}"
        SCORE=$(bc -l <<< "$SCORE+15")
else
        echo -e "  ${RED}Failed${NC}"
fi

cd $dir
echo -e "\nTesting :: cd ../../\n"
cat ./test-files/test_cd_A.txt ./test-files/test_exit.txt > ./test-files/cmd.txt
DIR=$(. ./test-files/test_cd_A.txt)
./shell < ./test-files/cmd.txt >temp 2>/dev/null
if [ $(grep -oF -- "${DIR}" temp | wc -l) -ge 3 ] && [ $(grep -oF -- "${DIR}/" temp | wc -l) -le 1 ]; then
        echo -e "  ${GREEN}Test Twelve Passed${NC}"
        SCORE=$(bc -l <<< "$SCORE+3")
else
        echo -e "  ${RED}Failed${NC}"
fi
rm temp

cd $dir
echo -e "\nTesting :: cd -\n"
cat ./test-files/test_cd_B.txt ./test-files/test_exit.txt > ./test-files/cmd.txt
DIR=$(pwd)
./shell < ./test-files/cmd.txt >temp 2>/dev/null
if cat temp | grep -oqF "${DIR}"; then
        echo -e "  ${GREEN}Test Thirteen Passed${NC}"
        SCORE=$(bc -l <<< "$SCORE+3")
else
        echo -e "  ${RED}Failed${NC}"
fi
rm temp

cd $dir
echo -e "\nTesting :: sleep 10\n"
cat ./test-files/sleep.txt ./test-files/test_exit.txt > ./test-files/cmd.txt
X=$(date +%s)
./shell < ./test-files/cmd.txt >/dev/null 2>&1
Y=$(date +%s)
diff=$((Y - X))
if (( diff < 12 )) && (( diff > 8 )); then
        echo -e "  ${GREEN}Test Fourteen Passed${NC}"
        SCORE=$(bc -l <<< "$SCORE+1")
else
        echo -e "  ${RED}Failed${NC}"
fi

cd $dir
echo -e "\nTesting :: Background sleep 10 &\n"
cat ./test-files/sleep_bg.txt ./test-files/test_exit.txt > ./test-files/cmd.txt
./shell < ./test-files/cmd.txt 2>/dev/null >temp
before=$(cat temp | grep -Eo '[0-9]{8,}' | head -n 1)
after=$(cat temp | grep -Eo '[0-9]{8,}' | tail -n 1)
diff=$((${after} - ${before}))
if (( diff < 2 )) && (( diff > -2 )); then
        echo -e "  ${GREEN}Test Fifteen Passed${NC}"
        SCORE=$(bc -l <<< "$SCORE+4")
else
        echo -e "  ${RED}Failed${NC}"
fi
rm temp

cd $dir
echo -e "\nTesting :: Zombies sleep 10 &\n"
cat ./test-files/sleep_bg.txt ./test-files/test_exit.txt > ./test-files/cmd.txt
before=$(ps -eo pid,ppid,cmd | grep sleep | grep -v grep | awk '$2 == 1')
./shell < ./test-files/cmd.txt >/dev/null 2>&1
after=$(ps -eo pid,ppid,cmd | grep sleep | grep -v grep | awk '$2 == 1')

if [[ "${before}" == "${after}" ]]; then
        echo -e "  ${GREEN}Test Sixteen Passed${NC}"
        SCORE=$(bc -l <<< "$SCORE+5")
else
        echo -e "  ${RED}Failed${NC}"
fi

make clean >/dev/null 2>&1

echo ""
echo ""
echo -e "${ORANGE}====================================${NC}"
echo -e "Net Score: ${GREEN}$SCORE/100.0${NC}"
echo -e "${ORANGE}====================================${NC}"

exit 0
