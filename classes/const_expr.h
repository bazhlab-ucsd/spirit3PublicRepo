#if __GNUC__ > 5
#define const_expr constexpr
#else
#define const_expr const
#endif
