#include "BasePCH.h"
#include "TextBox_HLSL.h"

enum WB_CPP_SYMBOLS
{
  SH_EOF,
  SH_UNKNOWN,
  SH_DELIMITER,
  SH_WHITESPACE,
  SH_OPERATOR,
  SH_LBRACKET,
  SH_RBRACKET,
  SH_LPAREN,
  SH_RPAREN,
  SH_KEYWORD,
  SH_FUNCTION,
  SH_VARIABLE,
  SH_NUMBER,
  SH_COMMA,
  SH_DOT,
  SH_TYPE,
  SH_DEFINE,
  SH_MACRO,
  SH_SEMICOLON,
};

enum WB_CPP_SYNTAXCOLORS
{
  SYNTAXHIGHLIGHT_COLOR_TEXT = 1,
  SYNTAXHIGHLIGHT_COLOR_COMMENT = 2,
  SYNTAXHIGHLIGHT_COLOR_KEYWORD = 3,
  SYNTAXHIGHLIGHT_COLOR_FUNCTION = 4,
  SYNTAXHIGHLIGHT_COLOR_DELIMITER = 5,
  SYNTAXHIGHLIGHT_COLOR_NUMBER = 6,
  SYNTAXHIGHLIGHT_COLOR_TYPE = 7,
  SYNTAXHIGHLIGHT_COLOR_VARIABLE = 8,
  SYNTAXHIGHLIGHT_COLOR_MACRO = 9,
};

void CWBTextBox_HLSL::OnDraw( CWBDrawAPI *API )
{
  CWBFont *Font = GetFont( GetState() );
  TS32 TabWidth = Font->GetWidth( _T( ' ' ) ) * 4;
  CColor Color = 0xff2b91af;
  //TS32 LineCount=0;

  //for (TS32 x=0; x<(TS32)Text.Length(); x++)
  //	if (Text[x]=='\n' || x==0) LineCount++;

  //CString s=CString::Format("%d",LineCount);
  //TS32 w=Font->GetWidth(s.GetPointer())+Font->GetWidth(' ');
  //SetClientPadding(w+1,1,1,1);

  CWBTextBox::OnDraw( API );

  CPoint Pos = CPoint( 0, 0 );
  CPoint Offset = -CPoint( GetHScrollbarPos(), GetVScrollbarPos() );

  API->SetCropRect( ClientToScreen( GetWindowRect() ) );

  TS32 LineCount = 0;

  for ( TS32 x = 0; x < (TS32)Text.Length(); x++ )
  {
    if ( Text[ x ] == '\n' || x == 0 )
    {
      LineCount++;
      CRect Display = CRect( CPoint( GetWindowRect().x1, Pos.y ), CPoint( 0, Pos.y + Font->GetLineHeight() ) ) + CPoint( 0, Offset.y );
      if ( Display.Intersects( GetWindowRect() ) )
      {
        CString s = CString::Format( "%d", LineCount );
        TS32 w = Font->GetWidth( s.GetPointer() ) + Font->GetWidth( ' ' );
        Font->Write( API, s, Pos - CPoint( w, -Offset.y ), Color );
      }
      Pos.y += Font->GetLineHeight();
    }
  }

}

CWBTextBox_HLSL::CWBTextBox_HLSL() : CWBTextBox()
{
  for ( TS32 x = 0; x < 256; x++ )
    Palette[ x ] = 0xffffffff;
}

CWBTextBox_HLSL::CWBTextBox_HLSL( CWBItem *Parent, const CRect &Pos, TS32 flags, const TCHAR *txt ) : CWBTextBox( Parent, Pos, flags, txt )
{
  for ( TS32 x = 0; x < 256; x++ )
    Palette[ x ] = 0xffffffff;

  SetPalette( SYNTAXHIGHLIGHT_COLOR_TEXT, 0xffb4b4b4 );
  SetPalette( SYNTAXHIGHLIGHT_COLOR_COMMENT, 0xff608b4e );
  SetPalette( SYNTAXHIGHLIGHT_COLOR_KEYWORD, 0xff569cd6 );
  SetPalette( SYNTAXHIGHLIGHT_COLOR_FUNCTION, 0xff00ff00 );
  SetPalette( SYNTAXHIGHLIGHT_COLOR_DELIMITER, 0xffb4b4b4 );
  SetPalette( SYNTAXHIGHLIGHT_COLOR_NUMBER, 0xffb5cea8 );
  SetPalette( SYNTAXHIGHLIGHT_COLOR_TYPE, 0xff00ffff );
  SetPalette( SYNTAXHIGHLIGHT_COLOR_VARIABLE, 0xfffff000 );
  SetPalette( SYNTAXHIGHLIGHT_COLOR_MACRO, 0xffbd63c5 );
  SetClientPadding( 1, 1, 1, 1 );
  SearchBox = NULL;
}

CWBTextBox_HLSL::~CWBTextBox_HLSL()
{

}

#define DELIMITERCOUNT 30
char Delimiters[ DELIMITERCOUNT ] = { '~', '!', '@', '%', '^', '&', '*', '(', ')', '-', '+', '=', '|', '\\', '/', '{', '}', '[', ']', ':', ';', '"', '\'', '<', '>', ' ', ',', '.', '?', '\n' };

#define KEYWORDCOUNT 85
char *KeyWords[ KEYWORDCOUNT ] =
{
  "blendstate", "break", "buffer", "cbuffer", "class", "compile", "const", "continue", "depthstencilstate", "depthstencilview", "discard", "do",
  "else", "extern", "false", "for", "geometryshader", "if", "in", "inline", "inout", "interface", "matrix", "namespace", "nointerpolation",
  "out", "pass", "pixelshader", "precise", "rasterizerstate", "rendrtargetview", "return", "register",
  "sampler_state", "samplercomparisonstate", "shared", "stateblock", "stateblock_state", "static", "string", "struct", "switch", "tbuffer",
  "technique", "technique10", "true", "typedef", "uniform", "vector", "vertexshader", "volatile", "while",

  //reserved:
  "auto", "case", "catch", "class", "const_cast", "default", "delete", "dynamic_cast", "enum", "explicit", "friend", "goto", "long", "mutable", "new",
  "operator", "private", "protected", "public", "reinterpret_cast", "short", "signed", "sizeof", "static_cast", "template", "this", "throw", "try", "typename",
  "union", "unsigned", "using", "virtual"
};

#define TYPECOUNT 33
char *TypeNames[ TYPECOUNT ] =
{
  "char", "bool", "double", "float", "half", "int", "sampler", "sampler1D", "sampler2D", "sampler3D", "samplerCUBE", "texture", "texture1D", "texture1Darray",
  "texture2D", "texture2Darray", "texture2DMS", "texture2DMSarray", "texture3D", "texturecube", "texturecubearray", "void", "float1", "float2", "float3", "float4", "float4x4", "float3x3", "Texture2D", "SamplerState",
  "int2", "int3", "int4"
};

#define FUNCTIONCOUNT 87

char *Functions[ FUNCTIONCOUNT ] =
{
  "abs", "acos", "all", "any", "asfloat", "asin", "asint", "asuint", "atan", "atan2", "ceil", "clamp", "clip", "cos", "cosh", "cross", "D3DCOLORtoUBYTE4", "ddx", "ddy",
  "degrees", "determinant", "distance", "dot", "exp", "exp2", "faceforward", "floor", "fmod", "frac", "frex", "fwidth", "GetRenderTargetSampleCount",
  "GetRenderTargetSamplePosition", "isfinite", "isinf", "isnan", "ldexp", "length", "lerp", "lit", "log", "log10", "log2", "max", "min", "modf", "mul", "noise",
  "normalize", "pow", "radians", "reflect", "refract", "round", "rsqrt", "saturate", "sign", "sin", "sincos", "sinh", "smoothstep", "sqrt", "step", "tan", "tanh",
  "tex1D", "tex1Dbias", "tex1Dgrad", "tex1Dlod", "tex1Dproj", "tex2D", "tex2Dbias", "tex2Dgrad", "tex2Dlod", "tex2Dproj", "tex3D", "tex3Dbias", "tex3Dgrad",
  "tex3Dlod", "tex3Dproj", "texCUBE", "texCUBEbias", "texCUBEgrad", "texCUBElod", "texCUBEproj", "transpose", "trunc"
};

#define MACROCOUNT 69

char *PredefinedMacros[ MACROCOUNT ] =
{
  "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7", "c8", "c9", "c10", "c11", "c12", "c13", "c14", "c15",
  "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
  "SV_TARGET0", "SV_TARGET1", "SV_TARGET2", "SV_TARGET3", "SV_TARGET4", "SV_TARGET5", "SV_TARGET6", "SV_TARGET7",
  "TEXCOORD0", "TEXCOORD1", "TEXCOORD2", "TEXCOORD3", "TEXCOORD4", "TEXCOORD5", "TEXCOORD6", "TEXCOORD7",
  "POSITION0", "POSITION1", "POSITION2", "POSITION3", "POSITION4", "POSITION5", "POSITION6", "POSITION7",
  "SV_POSITION",
  "NORMAL0", "NORMAL1", "NORMAL2", "NORMAL3", "NORMAL4", "NORMAL5", "NORMAL6", "NORMAL7",
  "COLOR0", "COLOR1", "COLOR2", "COLOR3"
};

TU32 CWBTextBox_HLSL::SkipSpace( TU32 Position ) //skips whitespace including comments
{
  if ( !isspace( (unsigned char)Text[ Position ] ) && ( Text[ Position ] != '/' || Position == Text.Length() - 1 ) ) return Position;

  if ( Position < Text.Length() - 1 && Text[ Position ] == '/' )
    if ( Text[ Position + 1 ] != '*' && Text[ Position + 1 ] != '/' ) return Position;

  TBOOL Done = false;
  do
  {
    if ( isspace( (unsigned char)Text[ Position ] ) ) Position++;
    else
    {
      if ( Text[ Position ] != '/' || Position == Text.Length() - 1 ) Done = true;
      else
      {
        if ( Text[ Position + 1 ] != '/' && Text[ Position + 1 ] != '*' ) Done = true;
        else
        {
          if ( Text[ Position + 1 ] == '/' )
          { //skip to the end of the line
            while ( Position < Text.Length() && Text[ Position ] != '\n' )
            {
              ColorString[ Position ] = SYNTAXHIGHLIGHT_COLOR_COMMENT;
              Position++;
            }
          }
          else
          { //skip to the first '*/'
            for ( ; Position < Text.Length(); Position++ )
            {
              ColorString[ Position ] = SYNTAXHIGHLIGHT_COLOR_COMMENT;
              if ( Text[ Position ] == '*' && Position < Text.Length() - 1 )
                if ( Text[ Position + 1 ] == '/' )
                {
                  Position++;
                  ColorString[ Position ] = SYNTAXHIGHLIGHT_COLOR_COMMENT;
                  break;
                }
            }

          }
        }
      }
    }

  } while ( !Done );

  return Position;
}

TBOOL isdelimiter( TCHAR c )
{
  for ( TS32 x = 0; x < DELIMITERCOUNT; x++ )
    if ( c == Delimiters[ x ] ) return true;
  return false;
}

TBOOL isstrstart( TCHAR *str, TS32 length, TCHAR *mask )
{
  if ( length != strlen( mask ) ) return false;
  while ( *mask )
  {
    if ( *str != *mask ) return false;
    str++;
    mask++;
  }
  return true;
}

TBOOL isstrstart( TCHAR *str, TS32 length, TCHAR *mask, TS32 mlength )
{
  if ( length != mlength ) return false;
  for ( TS32 x = 0; x < length; x++ )
  {
    if ( *str != *mask ) return false;
    str++;
    mask++;
  }
  return true;
}

void CWBTextBox_HLSL::DoSyntaxHighlight()
{

  struct LEXERTOKEN
  {
    TS32 Start;
    TS32 Length;
    WB_CPP_SYMBOLS Type;

    TS32 BracketLevel;
  };

  TS32 LineCount = 0;

  CWBFont *Font = GetFont( GetState() );

  for ( TS32 x = 0; x < (TS32)Text.Length(); x++ )
    if ( Text[ x ] == '\n' || x == 0 ) LineCount++;

  CString s = CString::Format( "%d", LineCount );
  TS32 w = Font->GetWidth( s.GetPointer() ) + Font->GetWidth( ' ' );
  SetClientPadding( w + 1, 1, 1, 1 );

  ColorString = Text; //sets the size to the correct size
  for ( TS32 x = 0; x < (TS32)ColorString.Length(); x++ )
    ColorString[ x ] = 0x01;

  //tokenization

  CArray<LEXERTOKEN> Tokens;

  TU32 Position = 0;
  while ( Position < Text.Length() )
  {
    //skip whitespace

    Position = SkipSpace( Position );
    if ( Position >= Text.Length() ) break;

    LEXERTOKEN t;
    t.Start = Position;
    t.Type = SH_UNKNOWN;

    //numbers first
    if ( isdigit( (unsigned char)Text[ Position ] ) )
    {
      //number
      while ( Position < Text.Length() && !isdelimiter( Text[ Position ] ) )
        ColorString[ Position++ ] = SYNTAXHIGHLIGHT_COLOR_NUMBER;
      t.Type = SH_NUMBER;
    }
    else
    {
      if ( isdelimiter( Text[ Position ] ) )
      {
        switch ( Text[ Position ] )
        {
        case '{': t.Type = SH_LBRACKET; break;
        case '}': t.Type = SH_RBRACKET; break;
        case '(': t.Type = SH_LPAREN; break;
        case ')': t.Type = SH_RPAREN; break;
        case ',': t.Type = SH_COMMA; break;
        case ';': t.Type = SH_SEMICOLON; break;
        case '+':
        case '-':
        case '*':
        case '//':
          t.Type = SH_OPERATOR; break;
        default: t.Type = SH_DELIMITER; break;
        }
        Position++;
      }
      else
        while ( Position < Text.Length() && !isspace( (unsigned char)Text[ Position ] ) && !isdelimiter( Text[ Position ] ) )
          Position++;
    }

    t.Length = Position - t.Start;
    Tokens += t;
  }

  //parse tokens
  TS32 BracketCount = 0;
  CArray<LEXERTOKEN> UserFunctions;
  CArray<LEXERTOKEN> Variables;
  CArray<LEXERTOKEN> Macros;

  //macros first
  for ( TS32 x = 0; x < Tokens.NumItems(); x++ )
  {
    if ( Tokens[ x ].Type == SH_UNKNOWN )
    {
      if ( isstrstart( Text.GetPointer() + Tokens[ x ].Start, Tokens[ x ].Length, "#define" ) )
      {
        Tokens[ x ].Type = SH_DEFINE;
        if ( x < Tokens.NumItems() - 1 )
          if ( Tokens[ x + 1 ].Type == SH_UNKNOWN )
          {
            Tokens[ x + 1 ].Type = SH_MACRO;
            Macros.Add( Tokens[ x + 1 ] );
          }
      }

      for ( TS32 y = 0; y < MACROCOUNT; y++ )
        if ( isstrstart( Text.GetPointer() + Tokens[ x ].Start, Tokens[ x ].Length, PredefinedMacros[ y ] ) )
        {
          Tokens[ x ].Type = SH_MACRO;
          break;
        }
      if ( Tokens[ x ].Type == SH_UNKNOWN )
      {
        for ( TS32 y = 0; y < Macros.NumItems(); y++ )
          if ( isstrstart( Text.GetPointer() + Tokens[ x ].Start, Tokens[ x ].Length, Text.GetPointer() + Macros[ y ].Start, Macros[ y ].Length ) )
          {
            Tokens[ x ].Type = SH_MACRO;
            break;
          }
      }
    }
  }

  for ( TS32 x = 0; x < Tokens.NumItems(); x++ )
  {
    if ( Tokens[ x ].Type == SH_LBRACKET ) BracketCount++;
    if ( Tokens[ x ].Type == SH_RBRACKET )
    {
      for ( TS32 y = 0; y < Variables.NumItems(); y++ )
        if ( Variables[ y ].BracketLevel >= BracketCount )
          Variables.DeleteByIndex( y-- );
      BracketCount--;
    }

    //find keywords
    if ( Tokens[ x ].Type == SH_UNKNOWN )
    {
      for ( TS32 y = 0; y < KEYWORDCOUNT; y++ )
        if ( isstrstart( Text.GetPointer() + Tokens[ x ].Start, Tokens[ x ].Length, KeyWords[ y ] ) )
        {
          Tokens[ x ].Type = SH_KEYWORD;
          break;
        }
    }

    //find variables
    if ( Tokens[ x ].Type == SH_UNKNOWN )
    {
      for ( TS32 y = 0; y < Variables.NumItems(); y++ )
        if ( isstrstart( Text.GetPointer() + Tokens[ x ].Start, Tokens[ x ].Length, Text.GetPointer() + Variables[ y ].Start, Variables[ y ].Length ) )
        {
          Tokens[ x ].Type = SH_VARIABLE;
          break;
        }
    }

    //functions
    if ( Tokens[ x ].Type == SH_UNKNOWN )
    {
      if ( BracketCount == 0 && x < Tokens.NumItems() - 1 && Tokens[ x + 1 ].Type == SH_LPAREN ) //add user function
      {
        Tokens[ x ].Type = SH_FUNCTION;
        UserFunctions += Tokens[ x ];
        TS32 pos = x + 2;

        //process function variables
        while ( pos < Tokens.NumItems() && ( Tokens[ pos ].Type == SH_TYPE || Tokens[ pos ].Type == SH_VARIABLE || Tokens[ pos ].Type == SH_UNKNOWN || Tokens[ pos ].Type == SH_COMMA || Tokens[ pos ].Type == SH_OPERATOR || Tokens[ pos ].Type == SH_DELIMITER ) )
        {
          pos++;
          if ( pos < Tokens.NumItems() && ( Tokens[ pos ].Type == SH_RPAREN || Tokens[ pos ].Type == SH_COMMA ) )
          {
            if ( Tokens[ pos - 1 ].Type == SH_UNKNOWN )
            {
              Tokens[ pos - 1 ].Type = SH_VARIABLE;
              Tokens[ pos - 1 ].BracketLevel = 1;
              Variables += Tokens[ pos - 1 ];
            }
          }
        }
      }

      //find functions
      if ( Tokens[ x ].Type == SH_UNKNOWN )
      {
        for ( TS32 y = 0; y < FUNCTIONCOUNT; y++ )
          if ( isstrstart( Text.GetPointer() + Tokens[ x ].Start, Tokens[ x ].Length, Functions[ y ] ) )
          {
            Tokens[ x ].Type = SH_FUNCTION;
            break;
          }

        if ( Tokens[ x ].Type == SH_UNKNOWN )
          for ( TS32 y = 0; y < UserFunctions.NumItems(); y++ )
            if ( isstrstart( Text.GetPointer() + Tokens[ x ].Start, Tokens[ x ].Length, Text.GetPointer() + UserFunctions[ y ].Start, UserFunctions[ y ].Length ) )
            {
              Tokens[ x ].Type = SH_FUNCTION;
              break;
            }
      }
    }

    //find types
    if ( Tokens[ x ].Type == SH_UNKNOWN )
    {
      for ( TS32 y = 0; y < TYPECOUNT; y++ )
        if ( isstrstart( Text.GetPointer() + Tokens[ x ].Start, Tokens[ x ].Length, TypeNames[ y ] ) )
        {
          Tokens[ x ].Type = SH_TYPE;
          if ( x < Tokens.NumItems() - 1 && Tokens[ x + 1 ].Type == SH_UNKNOWN )
          {
            if ( !( BracketCount == 0 && x < Tokens.NumItems() - 2 && Tokens[ x + 2 ].Type == SH_LPAREN ) )
            {
              Tokens[ x + 1 ].Type = SH_VARIABLE;
              Tokens[ x + 1 ].BracketLevel = BracketCount;
              Variables.Add( Tokens[ x + 1 ] );
            }
          }
          break;
        }
    }
  }

  //color string
  for ( TS32 x = 0; x < Tokens.NumItems(); x++ )
  {
    switch ( Tokens[ x ].Type )
    {
    case SH_KEYWORD: for ( TS32 y = 0; y < Tokens[ x ].Length; y++ ) ColorString[ y + Tokens[ x ].Start ] = SYNTAXHIGHLIGHT_COLOR_KEYWORD; break;
    case SH_TYPE: for ( TS32 y = 0; y < Tokens[ x ].Length; y++ ) ColorString[ y + Tokens[ x ].Start ] = SYNTAXHIGHLIGHT_COLOR_TYPE; break;
    case SH_FUNCTION: for ( TS32 y = 0; y < Tokens[ x ].Length; y++ ) ColorString[ y + Tokens[ x ].Start ] = SYNTAXHIGHLIGHT_COLOR_FUNCTION; break;
    case SH_MACRO: for ( TS32 y = 0; y < Tokens[ x ].Length; y++ ) ColorString[ y + Tokens[ x ].Start ] = SYNTAXHIGHLIGHT_COLOR_MACRO; break;
    case SH_VARIABLE: for ( TS32 y = 0; y < Tokens[ x ].Length; y++ ) ColorString[ y + Tokens[ x ].Start ] = SYNTAXHIGHLIGHT_COLOR_VARIABLE; break;
    }
  }
}

CWBItem * CWBTextBox_HLSL::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  TS32 Flags = 0;
  TS32 b = 0;
  node.GetAttributeAsInteger( _T( "singleline" ), &b );
  Flags |= b*WB_TEXTBOX_SINGLELINE;
  node.GetAttributeAsInteger( _T( "password" ), &b );
  Flags |= b*WB_TEXTBOX_PASSWORD;
  node.GetAttributeAsInteger( _T( "linenumbers" ), &b );
  Flags |= b*WB_TEXTBOX_LINENUMS;

  if ( node.HasAttribute( _T( "selection" ) ) )
  {
    node.GetAttributeAsInteger( _T( "selection" ), &b );
    Flags |= ( !b )*WB_TEXTBOX_NOSELECTION;
  }

  CWBTextBox_HLSL *textbox = new CWBTextBox_HLSL( Root, Pos, Flags );
  if ( node.HasAttribute( _T( "text" ) ) )
    textbox->SetText( node.GetAttribute( _T( "text" ) ) );

  return textbox;
}

TBOOL CWBTextBox_HLSL::MessageProc( CWBMessage &Message )
{

  switch ( Message.GetMessage() )
  {
  case WBM_KEYDOWN:
  {
    switch ( Message.Key )
    {
    case VK_ESCAPE:
      if ( SearchBox )
      {
        SAFEDELETE( SearchBox );
        UpdateSearch( CString( _T( "" ) ), false );
      }
      break;
    case VK_RETURN:
      if ( SearchBox && Message.GetTarget() == SearchBox->GetGuid() )
      {
        //UpdateSearch(SearchBox->GetText());
        return true;
      }
      break;
    case 'F':
      if ( Message.KeyboardState&WB_KBSTATE_CTRL )
      {
        StartSearch();
        return true;
      }
      break;
    case VK_F3:
      if ( SearchBox )
      {
        UpdateSearch( SearchBox->GetText(), true );
      }
      return true;
      break;
    }
  }
  break;

  case WBM_TEXTCHANGED:
  {
    if ( SearchBox && Message.GetTarget() == SearchBox->GetGuid() )
    {
      UpdateSearch( SearchBox->GetText(), false );
      return true;
    }
  }
  break;

  default:
    break;
  }

  return CWBTextBox::MessageProc( Message );
}

void CWBTextBox_HLSL::StartSearch()
{
  if ( !SearchBox )
  {
    CRect p = CRect( GetClientRect().TopRight() - CPoint( 150, 0 ), GetClientRect().TopRight() + CPoint( 0, 16 ) );
    SearchBox = new CWBTextBox( this, p, WB_TEXTBOX_SINGLELINE );
    SearchBox->ApplyStyleDeclarations( _T( "width:150px;height:16px;right:0px;top:0px;border:1px;background:#000000;padding:1px;" ) );
  }

  SearchBox->SetFocus();
  SearchBox->SetSelection( 0, SearchBox->GetText().Length() );
}

void CWBTextBox_HLSL::UpdateSearch( CString &str, TBOOL Next )
{
  SearchHighlightString = Text;

  for ( TU32 x = 0; x < SearchHighlightString.Length(); x++ )
    SearchHighlightString[ x ] = 0;

  if ( !str.Length() ) return;

  str.ToLower();
  CString s = Text;
  s.ToLower();

  TS32 found = 0;
  TS32 laststart = 0;
  do
  {
    found = s.Find( str, laststart );
    if ( found >= 0 )
    {
      for ( TU32 x = 0; x < str.Length(); x++ )
        SearchHighlightString[ found++ ] = 1;
      laststart = found;
    }
  } while ( found >= 0 );

  TS32 cursorp = s.Find( str, SearchCursorPos );
  if ( cursorp < 0 ) cursorp = s.Find( str );
  if ( cursorp >= 0 )
  {
    TS32 scp = SearchCursorPos;
    SetCursorPos( cursorp + str.Length(), false );
    if ( !Next ) SearchCursorPos = scp;
  }
}

CColor CWBTextBox_HLSL::GetTextColor( TS32 Index, CColor &DefaultColor )
{
  if ( Text.Length() == ColorString.Length() ) return Palette[ ColorString[ Index ] & 0xff ];
  return CColor::FromARGB( 0xffffffff );
}

TBOOL CWBTextBox_HLSL::GetTextBackground( TS32 Index, CColor &Result )
{
  if ( Text.Length() != SearchHighlightString.Length() ) return false;
  if ( !SearchHighlightString[ Index ] ) return false;

  Result = CColor::FromARGB( 0x80ffffff );

  if ( SearchBox )
  {
    if ( Index < SearchCursorPos && Index >= (TS32)( SearchCursorPos - SearchBox->GetText().Length() ) )
      Result = CColor::FromARGB( 0x80ff8080 );
  }

  return true;
}

void CWBTextBox_HLSL::OnCursorPosChange( TS32 Cpos )
{
  SearchCursorPos = Cpos;
}

void CWBTextBox_HLSL::PostCharInsertion( TS32 CursorPos, TS32 Key )
{
  TS32 bracketcount = 0;
  for ( TS32 x = 0; x < CursorPos; x++ )
  {
    if ( Text[ x ] == '{' ) bracketcount++;
    if ( Text[ x ] == '}' ) bracketcount--;
  }

  TS32 tab = _T( '\t' );

  if ( Key == VK_RETURN ) //insert padding tabs
  {
    for ( TS32 x = 0; x < bracketcount; x++ )
      InsertText( CursorPos + x, (TCHAR*)&tab, 1, CursorPos + x + 1 );

    return;
  }

  if ( Key == _T( '}' ) ) //if on empty line, place to proper tab depth
  {
    TS32 linestart = CursorPos;

    do
    {
      linestart--;
    } while ( linestart >= 0 && Text[ linestart ] != _T( '\n' ) );
    linestart++;

    for ( TS32 x = linestart; x < CursorPos - 1; x++ )
      if ( !_istspace( Text[ x ] ) ) return;

    RemoveText( linestart, CursorPos - 1 - linestart, linestart );
    for ( TS32 x = 0; x < bracketcount; x++ )
      InsertText( linestart + x, (TCHAR*)&tab, 1, linestart + x + 1 );

    return;
  }
}
