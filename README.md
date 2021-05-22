# CLE-Assigment2

## PART1

### COMPILE

```bash
$ mpicc -Wall -o main main.c convertchar.c dispatcher.c worker.c
```

### RUN

```bash
$ mpiexec -n 2 ./main -f "dataset/text0.txt dataset/text1.txt  dataset/text2.txt dataset/text3.txt dataset/text4.txt"
```

## PART2

### COMPILE

```bash
$ mpicc -Wall -o main main.c dispatcher.c worker.c
```

### RUN

```bash
$ mpiexec -n 2 ./main -f "dataset/newSigVal01.bin dataset/newSigVal02.bin dataset/newSigVal03.bin dataset/newSigVal04.bin"
```
