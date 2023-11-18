#pragma once
#include "../BaseLib/BaseLib.h"

//#import "msxml3.dll" named_guids raw_interfaces_only
#include "msxml3.tlh"

using namespace MSXML2;

class CXMLDocument;

class CXMLNode
{
public:
  CXMLNode( void );
  CXMLNode( const CXMLNode &Original );
  CXMLNode( MSXML2::IXMLDOMNode *, CXMLDocument *, TS32 );
  virtual ~CXMLNode();

  CXMLNode operator=( const CXMLNode Original );

  TS32 GetChildCount();
  TS32 GetChildCount( TCHAR * );
  CXMLNode GetChild( TS32 );
  CXMLNode GetChild( TCHAR * );
  CXMLNode GetChild( TCHAR *, TS32 );

  TS32 IsValid();

  CString GetNodeName();

  void GetText( TCHAR*, TS32 );
  CString GetText();
  TBOOL GetValue( TS32 &Int );
  TBOOL GetValue( TBOOL &Int );
  TBOOL GetValue( TF32 &Float );

  TBOOL GetAttribute( TCHAR * szAttribute, TCHAR * szBuffer, TS32 nBufferSize );
  CString GetAttribute( TCHAR * szAttribute );
  void GetAttributeAsInteger( TCHAR * szAttribute, TS32 * nValue );
  void GetAttributeAsFloat( TCHAR * szAttribute, TF32 * fValue );
  CString GetAttributeAsString( TCHAR * szAttribute );
  TBOOL HasAttribute( TCHAR * szAttribute );

  //void FlushNode();
  CXMLNode AddChild( TCHAR*, TBOOL PostEnter = true );
  void SetText( const TCHAR* );
  void SetText( CString &s );
  void SetInt( TS32 Int );
  void SetFloat( TF32 Float );
  void SetAttribute( TCHAR * szAttributeName, const TCHAR * szValue );
  void SetAttributeFromInteger( TCHAR * szAttributeName, TS32 nValue );
  void SetAttributeFromFloat( TCHAR * szAttributeName, TF32 nValue );
  CString SaveToString();

  MSXML2::IXMLDOMNode * GetNode() { return pNode; }

private:
  TS32 nLevel;
  MSXML2::IXMLDOMNode * pNode;
  //MSXML2::IXMLDOMDocument * pDoc;
  CXMLDocument *pDoc;
};