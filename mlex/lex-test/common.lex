
%%
\+                  {return 1;}
\-                  {return 2;}
\*					{return 3;}
\/					{return 4;}
\%					{return 5;}
\>					{return 6;}
\<					{return 7;}
\>\=					{return 8;}
\<\=					{return 9;}
\=\=					{return 10;}
\>\>					{return 11;}
\<\<					{return 12;}
\"(.)*\"					{return 13;}
if					{return 14;}
else					{return 15;}
do					{return 16;}
while					{return 17;}
until					{return 18;}
void					{return 19;}
int					{return 20;}
long					{return 21;}
([a-zA-Z]|\_)([a-zA-Z]|\_|[0-9])*					{return 22;}
0[xX][0-9A-F]+					{return 23;}
[1-9][0-9]*					{return 24;}