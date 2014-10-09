#include <xinu.h>
 
void prchP(char c, char d);
void prchT(char c, char d);
void prchQ(char c, char d);
int prA, prB, prC;
int round;
unsigned long ctr100;
int main(int argc, char **argv){
  round = 100;
  resume( prA = create(prchP, 2000, PROPORTIONALSHARE, 20, "proc A", 2, 'A', 'A') );
  resume( prB = create(prchP, 2000, PROPORTIONALSHARE, 50, "proc B", 2, 'B', 'B') );
  resume( prC = create(prchT, 2000, TSSCHED, 15, "proc C", 2, 'C', 'C') );

  while (1) {
  sleepms(10000); 
  }
  return OK;
}
void prchP(char c, char d){
  sleepms(500);
  kprintf("Proc %c starts!\r\n", c);
  int i;
  for(i=0; i<round; i++){
    kprintf("%c:%d", d,i);
}
  kprintf("Proc %c ends! \r\n", c);
}

void prchQ(char c, char d){
  kprintf("Proc %c starts!\r\n", c);
  int i;
  for(i=0; i<round; i++){
    kprintf("%c:%d", d,i);
}
  kprintf("Proc %c ends! \r\n", c);
}

void prchT(char c, char d){
  sleepms(500);
  kprintf("Proc %c starts!\r\n", c);
  int i;
  for(i=0; i<round; i++){
    kprintf("%c:%d", d,i);
    if((i+1)%50 == 0) sleepms(20);
  }
  kprintf("Proc %c ends! \r\n", c);
}
