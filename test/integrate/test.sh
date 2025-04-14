for i in `seq 1 20`; do
    rm -rf *.json
    rm -rf *.sealed
    rm -rf *.output
    ../../bin/test_scheduler
    echo "Test $i"
    echo "Test $i"
    echo "Test $i"
    echo "Test $i"
    echo "Test $i"
done
