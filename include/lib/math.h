#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MODULO_ADD(x, i, m) (((x) + (i)) % (m))
// nimmt an, dass x, i <= m sind
#define MODULO_SUB(x, i, m) ((x) >= (i) ? (x) - (i) : (m) - (i) + (x))

#define IS_POWER_OF_TWO(x) (((x) & ((x)-1)) == 0)