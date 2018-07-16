
%%

break									{return JsTokenType::Key_break;}
case									{return JsTokenType::Key_case;}
catch									{return JsTokenType::Key_catch;}
class									{return JsTokenType::Key_class;}
const									{return JsTokenType::Key_const;}
continue								{return JsTokenType::Key_continue;}
default									{return JsTokenType::Key_default;}
delete									{return JsTokenType::Key_delete;}
do										{return JsTokenType::Key_do;}
else									{return JsTokenType::Key_else;}
export									{return JsTokenType::Key_export;}
finally									{return JsTokenType::Key_finally;}
for										{return JsTokenType::Key_for;}
function								{return JsTokenType::Key_function;}
if										{return JsTokenType::Key_if;}
import									{return JsTokenType::Key_import;}
in										{return JsTokenType::Key_in;}
new										{return JsTokenType::Key_new;}
return									{return JsTokenType::Key_return;}
super									{return JsTokenType::Key_super;}
switch									{return JsTokenType::Key_switch;}
this									{return JsTokenType::Key_this;}
throw									{return JsTokenType::Key_throw;}
try										{return JsTokenType::Key_try;}
typeof									{return JsTokenType::Key_typeof;}
var										{return JsTokenType::Key_var;}
void									{return JsTokenType::Key_void;}
while									{return JsTokenType::Key_while;}

null									{return JsTokenType::NullLiteral;}
true|false								{return JsTokenType::BooleanLiteral;}
\/										{return JsTokenType::DivPunctuator;}
\}										{return JsTokenType::RightBracePunctuator;}
\*|\/|\%								{return JsTokenType::MultiplicativeOperator;}
\"*\"									{return JsTokenType::DoubleStringLiteral;}
\'*\'									{return JsTokenType::SingleStringLiteral;}
\\\\*									{return JsTokenType::SingleLineComment;}
0|([1-9][0-9]*)							{return JsTokenType::DecimalLiteral;}
0[bB][01]*								{return JsTokenType::BinaryIntegerLiteral;}
0[oO][0-7]*								{return JsTokenType::OctalIntegerLiteral;}
0[xX][0-9a-fA-F]*						{return JsTokenType::HexIntegerLiteral;}
[eE](\+|\-)[0-9]*						{return JsTokenType::ExponentPart;}
(\$|\_|[a-zA-Z])([0-9a-zA-Z]*|\$|\_)	{return JsTokenType::IdentifierName;}