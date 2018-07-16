
#include "MpToken.h"

using namespace MParser;

%%

yield							{return MpTokenType::TOKEN_YIELD;}
await							{return MpTokenType::TOKEN_AWAIT;}
this							{return MpTokenType::TOKEN_THIS;}
\(								{return MpTokenType::TOKEN_S_LBRACKET;}
\)								{return MpTokenType::TOKEN_S_RBRACKET;}
\[								{return MpTokenType::TOKEN_M_LBRACKET;}
\]								{return MpTokenType::TOKEN_M_RBRACKET;}
\{								{return MpTokenType::TOKEN_L_LBRACKET;}
\}								{return MpTokenType::TOKEN_L_RBRACKET;}
\+\+							{return MpTokenType::TOKEN_INCREMENT;}
\-\-							{return MpTokenType::TOKEN_DECREMENT;}
\<\=							{return MpTokenType::TOKEN_SMALLER_EQUAL;}
\>\=							{return MpTokenType::TOKEN_GREATER_EQUAL;}
\&\&							{return MpTokenType::TOKEN_L_AND;}
\|\|							{return MpTokenType::TOKEN_L_OR;}
\+								{return MpTokenType::TOKEN_PLUS;}
\-								{return MpTokenType::TOKEN_SUB;}
\~								{return MpTokenType::TOKEN_A_NOT;}
\!								{return MpTokenType::TOKEN_L_NOT;}
\*								{return MpTokenType::TOKEN_MUL;}
\/								{return MpTokenType::TOKEN_DIV;}
\%								{return MpTokenType::TOKEN_MOD;}
\>								{return MpTokenType::TOKEN_GREATER;}
\<								{return MpTokenType::TOKEN_SMALLER;}
\&								{return MpTokenType::TOKEN_A_AND;}
\^								{return MpTokenType::TOKEN_XOR;}
\|								{return MpTokenType::TOKEN_A_OR;}
\.								{return MpTokenType::TOKEN_DOT;}
\,								{return MpTokenType::TOKEN_COMMA;}
\=								{return MpTokenType::TOKEN_ASSIGNMENT;}
\:								{return MpTokenType::TOKEN_PRODUCTION;}
new								{return MpTokenType::TOKEN_NEW;}
super							{return MpTokenType::TOKEN_SUPER;}
delete							{return MpTokenType::TOKEN_DELETE;}
void							{return MpTokenType::TOKEN_VOID;}
typeof							{return MpTokenType::TOKEN_TYPEOF;}
instanceof						{return MpTokenType::TOKEN_INSTANCEOF;}
in								{return MpTokenType::TOKEN_IN;}

IdentifierName					{return MpTokenType::TOKEN_IdentifierName;}

[A-Z][a-zA-Z]*					{return MpTokenType::TOKEN_UNTERMINATOR;}
