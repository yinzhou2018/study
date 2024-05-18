%%
server:
  | SERVER OP exp_list CP
  ;

exp_list:
  | subexp exp_list
  | exp TERMINATOR exp_list
  ;

subexp: 
  LOCATION path OP exp_list CP
  ;

exp: keyword value
  ;

path: VALUE

keyword: LISTEN
  | NAME {printf("\t%s:", "name");}
  | ALOG {printf("\t%s:", "access_log");}
  | ELOG {printf("\t%s:", "error_log");}
  | PROXY_PASS {printf("\t%s:", "proxy-pass");}
  | INCLUDE {printf("\t%s - need to read file and put it here: ", "include");}

value: VALUE
%%