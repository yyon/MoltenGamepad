#include "../moltengamepad.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include "../parser.h"

void print_profile(profile& profile, std::ostream& out) {
  profile.lock.lock();
  for (auto it = profile.mapping.begin(); it != profile.mapping.end(); it++) {
    out << profile.name << "." << it->first << " = ";
    MGTransDef def;
    it->second.trans->fill_def(def);
    MGparser::print_def(it->second.type, def, out);
    out << std::endl;
  }
  for (auto entry : profile.adv_trans) {
    out << profile.name << ".(" << entry.first << ") = ";
    MGTransDef def;
    entry.second.trans->fill_def(def);
    MGparser::print_def(NO_ENTRY, def, out);
    out << std::endl;
  }
  for (auto it = profile.options.begin(); it != profile.options.end(); it++) {
    out << profile.name << ".?" << it->first << " = " << it->second.stringval << std::endl;
    out << "   #  " << it->second.descr << std::endl;
  }
  if (profile.mapping.empty() && profile.adv_trans.empty() && profile.options.empty())
    out << "#(empty profile)" << std::endl;
  profile.lock.unlock();
}

int do_print_profile(moltengamepad* mg, std::string name, std::ostream& out) {
  if (name.empty()) {
    mg->for_all_profiles([&out] (auto prof) { out << prof->name << std::endl; });
    return 0;
  }

  auto prof = mg->find_profile(name);
  if (prof) {
    print_profile(*(prof.get()), out);
    return 0;
  }
  return -1;

}

void print_driver_dev_list(device_manager* man, std::ostream& out) {
  if (!man) return;
  out << "(" << man->name << ")\n";
  bool has_dev = false;
  auto print_dev = [&out, &has_dev] (const input_source* dev) {
    out << dev->get_name() << ":\t" << dev->get_description() << std::endl;
    has_dev = true;
  };

  man->for_all_devices(print_dev);

  if (!has_dev) {
    out << "  no devices" << std::endl;
  }
  
}

int do_print_devs(moltengamepad* mg, std::string name, std::ostream& out) {
  if (!name.empty()) {
    std::shared_ptr<input_source> dev = mg->find_device(name.c_str());
    if (dev.get()) {
      out << dev->get_name() <<std::endl;
      out << " \"" << dev->get_description() << "\"" << std::endl;
      out << " type: " << dev->get_type() << std::endl;
      auto uniq = dev->get_uniq();
      if (!uniq.empty()) out << " uniq: \"" << uniq << "\"" << std::endl;

      out << " events:" << std::endl;
      
      const std::vector<source_event>& events = dev->get_events();
      for (auto v : events) {
        const char* evstate = (v.state == EVENT_INACTIVE) ? " (inactive)" : "";
        if (v.state != EVENT_DISABLED) out << v.name << ":\t" << v.descr << evstate << std::endl;
      }

      std::vector<option_info> list;
      dev->list_options(list);
      out << " options:" << std::endl;
      for (auto v : list) {
        out << "?" << v.name << " = " << v.stringval << "\n\t" << v.descr << std::endl;
      }
      if (list.empty()) {
        out << "(no options)" << std::endl;
      }

      return 0;
    }

    return -1;
  }

  for (auto man : mg->managers) {
    print_driver_dev_list(man, out);
  }

  return 0;
}

int do_print_drivers(moltengamepad* mg, std::string name, std::ostream& out) {
  if (name.empty()) {
    for (auto man : mg->managers) {
      out << man->name <<  std::endl;
    }
    return 0;
  }
  device_manager* man = mg->find_manager(name.c_str());
  if (man) {
    print_driver_dev_list(man, out);
  }

  return 0;

}

int do_print_slots(moltengamepad* mg, std::string name, std::ostream& out) {
  if (name.empty()) {
    for (auto slot : mg->slots->slots) {
      out << slot->name << ":\t" << slot->descr <<  std::endl;
    }
    out << mg->slots->keyboard->name << ":\t" << mg->slots->keyboard->descr << std::endl;
    out << mg->slots->dummyslot->name << ":\t" << mg->slots->dummyslot->descr << std::endl;
    if (mg->slots->debugslot) out << mg->slots->debugslot->name << ":\t" << mg->slots->debugslot->descr << std::endl;
    return 0;
  }
  output_slot* slot = mg->slots->find_slot(name);
  if (slot) {
    out << slot->name << ":\t" << slot->descr << std::endl;
    for (auto e : slot->options) {
      out << "\t" << e.first << " = " << e.second << std::endl;
    }
  }

  return 0;

}

#define PRINT_USAGE ""\
"USAGE:\n\tprint <type> [element]\n"\
"\ttypes recognized: drivers, devices, profiles, slots\n"\
"\tprint <type> will list all elements of that type\n"\
"\tprint <type> [element] will show detailed info on that element\n"
int do_print(moltengamepad* mg, std::vector<token>& command) {
  if (command.size() < 2) {
    std::cout << PRINT_USAGE << std::endl;
    return -1;
  }
  std::string arg = (command.size() >= 3 && command.at(2).type == TK_IDENT) ? command.at(2).value : "";
  if (command.at(1).value.compare(0, 6, "driver") == 0) return do_print_drivers(mg, arg, std::cout);
  if (command.at(1).value.compare(0, 6, "device") == 0) return do_print_devs(mg, arg, std::cout);
  if (command.at(1).value.compare(0, 7, "profile") == 0) return do_print_profile(mg, arg, std::cout);
  if (command.at(1).value.compare(0, 4, "slot") == 0) return do_print_slots(mg, arg, std::cout);

  std::cout << PRINT_USAGE << std::endl;
  return 0;
}


