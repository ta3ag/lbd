#include "ch_Composite.h"

extern "C" int printf(const char *format, ...);

Equipment::Equipment(const char* name) {
  _name = name;
}

CompositeEquipment::CompositeEquipment(const char* name)
  : Equipment(name) {
}

#if 0
void CompositeEquipment::Add (Equipment* equip) {
  _equipment.Add(equip);
}

Currency CompositeEquipment::NetPrice () {
  Iterator<Equipment*>* i = CreateIterator();
  Currency total = 0;
  for (i->First(); !i->IsDone(); i->Next()) {
    total += i->CurrentItem()->NetPrice();
  }
//  delete i;
  return total;
}
#endif
#if 0
Iterator<Equipment*>* CompositeEquipment::CreateIterator() {
  Iter = new ListIterator<Equipment*>(&_equipment);
  return Iter;
}
#endif

Chassis::Chassis(const char* name) 
  : CompositeEquipment(name) {
}

#if 1
int main() {
  Cabinet cabinet("PC Cabinet");
#if 0
  Chassis chassis("PC Chassis");
  cabinet.Add(&chassis);
  Bus bus("MCA Bus");
  Card card("16Mbs Token Ring");
  bus.Add(&card);
  chassis.Add(&bus);
  FloppyDisk floppyDisk("3.5in Floppy");
  chassis.Add(&floppyDisk);
//  cout << "The net price is " << chassis->NetPrice() << endl;
  printf("The net price is %d\n", chassis.NetPrice());
#endif
}
#endif

