# Deterministic `PTHREAD`

Esta biblioteca dinámica intercepta llamadas a `pthread` con el fin de ajustar la prioridad 
de los hilos RT (`SCHED_FIFO` y `SCHED_RR`) y asignarles afinidad a determinados procesadores.

## Requisitos

- `gcc`

## Clonación

```bash
git clone https://github.com/deterministic-spa/deterministic-pthread.git
cd deterministic-pthread
```

## Compilación

```bash
gcc -shared -fPIC -o build/libdeterministic_pthread.so src/deterministic_pthread.c -ldl -pthread
```

## Uso

```bash
LD_PRELOAD=build/libdeterministic_pthread.so \ 
    DET_PTHREAD_CPU_SET=<conjunto_cpus> \
    DET_PTHREAD_PRIORITY=<prioridad> \
    <comando>
```

### Ejemplo

```bash
LD_PRELOAD=build/libdeterministic_pthread.so \
    DET_PTHREAD_CPU_SET=2,3 \
    DET_PTHREAD_PRIORITY=80 \
    jackd ...
```
