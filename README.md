# CLE-Assigment2

## PART1

### COMPILE

```bash
$ mpicc -Wall -o main main.c convertchar.c dispatcher.c worker.c
```

### RUN

```bash
$ mpiexec -n 2 ./main -f "dataset/text0.txt"
```
