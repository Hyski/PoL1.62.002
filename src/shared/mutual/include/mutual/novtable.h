#if !defined(__NOVTABLE_H_INCLUDED_5992692519605117__)
#define __NOVTABLE_H_INCLUDED_5992692519605117__

// ќбъ€вим макрос, дл€ убирани€ ссылки на vtable интерфейсных классов
//   мотиваци€: уменьшение времени инициализации наследованных классов,
//              уменьшение объема кода

#if defined(_MSC_VER) || defined(__ICL)
#define ML_NOVTABLE		__declspec(novtable)
#else
#define ML_NOVTABLE
#endif

#endif // !defined(__NOVTABLE_H_INCLUDED_5992692519605117__)