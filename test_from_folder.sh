#!/usr/bin/bash

# Path to minisat
export MINISAT=~/minisat/simp/minisat
# Path to sat
export SAT=~/sr/sat-solver/sat
# Where to find the cnf files
export CNF_FILES=~/sr/tests/
# Output path
export TEST_PATH=~/test-results
mkdir -p $TEST_PATH/minisat $TEST_PATH/sat 
find $CNF_FILES -name "*.cnf" -exec $MINISAT {} {}.res \;
mv $CNF_FILES/*.res $TEST_PATH/minisat
find $CNF_FILES -name "*.cnf" -printf "$SAT %p &> %p.res\n" | sh 
mv $CNF_FILES/*.res $TEST_PATH/sat

COUNT=$(ls -l $TEST_PATH/minisat/*.res | wc -l)
num=0
for x in `ls $TEST_PATH/minisat/`;
do
  echo "Comparing $x"
  miniline=$(head -n 1 $TEST_PATH/minisat/$x)
  satline=$(head -n 1 $TEST_PATH/sat/$x)
  # This line should be SAT or UNSAT
  if [ "$miniline" == "$satline" ];
  then
    echo "$x passed"
    num=$(( num + 1 ))
  else
    echo "$x didn't pass expected: $miniline got: $satline"
  fi

done
echo "Correct result = $num / $COUNT"
