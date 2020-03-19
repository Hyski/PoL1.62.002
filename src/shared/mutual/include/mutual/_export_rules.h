#if !defined(___EXPORT_RULES_H_INCLUDED_5992692519605117__)
#define ___EXPORT_RULES_H_INCLUDED_5992692519605117__

#if !defined(MUTUAL_STATIC)
#	if !defined(MUTUAL_API)
#		define MUTUAL_API __declspec(dllimport)
#	endif
#else
#	ifndef MUTUAL_API
#		define MUTUAL_API
#	endif
#endif

#endif // !defined(___EXPORT_RULES_H_INCLUDED_5992692519605117__)