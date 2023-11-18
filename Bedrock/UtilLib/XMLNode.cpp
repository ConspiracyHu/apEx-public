#include "XMLNode.h"
#include "XMLDocument.h"

CXMLNode::CXMLNode()
{
  pNode = NULL;
  pDoc = NULL;
  nLevel = 0;
}

CXMLNode::CXMLNode( MSXML2::IXMLDOMNode * p, CXMLDocument * d, TS32 l )
{
  pNode = p;
  pDoc = d;
  nLevel = l;
}

CXMLNode::CXMLNode( const CXMLNode &Original )
{
  nLevel = Original.nLevel;
  pNode = Original.pNode;
  if ( pNode ) pNode->AddRef();
  pDoc = Original.pDoc;
}

CXMLNode CXMLNode::operator=( const CXMLNode Original )
{
  if ( pNode ) pNode->Release();

  nLevel = Original.nLevel;
  pNode = Original.pNode;
  if ( pNode ) pNode->AddRef();
  pDoc = Original.pDoc;
  return *this;
}


CXMLNode::~CXMLNode()
{
  if ( pNode )
    pNode->Release();
}

TS32 CXMLNode::GetChildCount()
{
  MSXML2::IXMLDOMNode * pChild = NULL;
  pNode->get_firstChild( &pChild );
  //  if (!pChild) return 0;
  TS32 i = 0;
  while ( pChild )
  {
    i++;
    MSXML2::IXMLDOMNode * pSibling = NULL;
    pChild->get_nextSibling( &pSibling );
    pChild->Release();
    pChild = pSibling;
  }

  if ( pChild ) pChild->Release();
  return i;
}

TS32 CXMLNode::GetChildCount( TCHAR * szNodeName )
{
  MSXML2::IXMLDOMNode * pChild = NULL;
  pNode->get_firstChild( &pChild );
  // if (!pChild) return 0;
  TS32 i = 0;

#ifndef UNICODE
	TS32 Size = (TS32)strlen(szNodeName);
	wchar_t *name = new wchar_t[Size + 2];
	name[Size + 1] = 0;
	if (!MultiByteToWideChar(CP_UTF8, 0, szNodeName, -1, name, Size + 1))
	{
		SAFEDELETEA(name);
		return 0;
	}
#else
	wchar_t *name = szNodeName;
#endif

  while ( pChild )
  {
    WCHAR * bName;
    pChild->get_nodeName( &bName );
    if ( wcscmp( bName, name ) == 0 )
      //if (nodename == bName)
      i++;

    SysFreeString( bName );

    MSXML2::IXMLDOMNode * pSibling = NULL;
    pChild->get_nextSibling( &pSibling );
    pChild->Release();
    pChild = pSibling;
  }

#ifndef UNICODE
  SAFEDELETEA( name );
#endif

  if ( pChild ) pChild->Release();
  return i;
}

CString CXMLNode::GetNodeName()
{
  WCHAR * szName;
  pNode->get_nodeName( &szName );

  CString s = szName;
  SysFreeString( szName );

  return s;
}

CXMLNode CXMLNode::GetChild( TS32 n )
{
  MSXML2::IXMLDOMNode * pChild = NULL;
  pNode->get_firstChild( &pChild );
  TS32 i = 0;
  while ( pChild )
  {
    if ( n == i )
      return CXMLNode( pChild, pDoc, nLevel + 1 );
    i++;
    MSXML2::IXMLDOMNode * pSibling = NULL;
    pChild->get_nextSibling( &pSibling );
    pChild->Release();
    pChild = pSibling;
  }
  if ( pChild ) pChild->Release();
  return CXMLNode();
}

CXMLNode CXMLNode::GetChild( TCHAR * szNodeName )
{
  MSXML2::IXMLDOMNode * pChild = NULL;
  pNode->get_firstChild( &pChild );
  TS32 i = 0;

#ifndef UNICODE
	TS32 Size = (TS32)strlen(szNodeName);
	wchar_t *name = new wchar_t[Size + 2];
	name[Size + 1] = 0;
	if (!MultiByteToWideChar(CP_UTF8, 0, szNodeName, -1, name, Size + 1))
	{
		SAFEDELETEA(name);
		return CXMLNode();
	}
#else
	wchar_t *name = szNodeName;
#endif

  while ( pChild )
  {
    WCHAR * bName;
    pChild->get_nodeName( &bName );
    if ( wcscmp( bName, name ) == 0 )
    {
#ifndef UNICODE
      SAFEDELETEA( name );
#endif
      SysFreeString( bName );
      return CXMLNode( pChild, pDoc, nLevel + 1 );
    }
    i++;
    MSXML2::IXMLDOMNode * pSibling = NULL;
    pChild->get_nextSibling( &pSibling );
    pChild->Release();
    pChild = pSibling;
    SysFreeString( bName );
  }
#ifndef UNICODE
  SAFEDELETEA( name );
#endif
  if ( pChild ) pChild->Release();
  return CXMLNode();
}

CXMLNode CXMLNode::GetChild( TCHAR * szNodeName, TS32 n )
{
  MSXML2::IXMLDOMNode * pChild = NULL;
  pNode->get_firstChild( &pChild );
  TS32 i = 0;

#ifndef UNICODE
	TS32 Size = (TS32)strlen(szNodeName);
	wchar_t *name = new wchar_t[Size + 2];
	name[Size + 1] = 0;
	if (!MultiByteToWideChar(CP_UTF8, 0, szNodeName, -1, name, Size + 1))
	{
		SAFEDELETEA(name);
		return CXMLNode();
	}
#else
	wchar_t *name = szNodeName;
#endif

  while ( pChild )
  {
    WCHAR * bName;
    pChild->get_nodeName( &bName );
    if ( wcscmp( bName, name ) == 0 )
    {
      if ( n == i )
      {
#ifndef UNICODE
        SAFEDELETEA( name );
#endif
        SysFreeString( bName );
        return CXMLNode( pChild, pDoc, nLevel + 1 );
      }
      i++;
    }
    MSXML2::IXMLDOMNode * pSibling = NULL;
    pChild->get_nextSibling( &pSibling );
    pChild->Release();
    pChild = pSibling;
    SysFreeString( bName );
  }

#ifndef UNICODE
  SAFEDELETEA( name );
#endif

  if ( pChild ) pChild->Release();
  return CXMLNode();
}

void CXMLNode::GetText( TCHAR * szBuffer, TS32 nBufferSize )
{
  WCHAR * bStr;
  pNode->get_text( &bStr );
  CString s = bStr;
  _tcsncpy_s( szBuffer, nBufferSize, s.GetPointer(), _TRUNCATE );
  SysFreeString( bStr );
}
CString CXMLNode::GetText()
{
  WCHAR * bStr;
  pNode->get_text( &bStr );
  CString s = CString( bStr );
  SysFreeString( bStr );
  return s;
}

TBOOL CXMLNode::GetAttribute( TCHAR * szAttribute, TCHAR * szBuffer, TS32 nBufferSize )
{
  MSXML2::IXMLDOMNamedNodeMap * pNodeMap = NULL;
  MSXML2::IXMLDOMNode * pAttribMap = NULL;

  pNode->get_attributes( &pNodeMap );
  pNodeMap->nextNode( &pAttribMap );

#ifndef UNICODE
	TS32 Size = (TS32)strlen(szAttribute);
	wchar_t *name = new wchar_t[Size + 2];
	name[Size + 1] = 0;
	if (!MultiByteToWideChar(CP_UTF8, 0, szAttribute, -1, name, Size + 1))
	{
		SAFEDELETEA(name);
		return false;
	}
#else
	wchar_t *name = szAttribute;
#endif

  while ( pAttribMap )
  {
    WCHAR * bName;
    VARIANT vValue;
    pAttribMap->get_nodeName( &bName );
    pAttribMap->get_nodeValue( &vValue );
    if ( wcscmp( bName, name ) == 0 )
    {
      if ( pAttribMap ) pAttribMap->Release();
      if ( pNodeMap ) pNodeMap->Release();
      CString v = vValue.bstrVal;
      _tcsncpy_s( szBuffer, nBufferSize, v.GetPointer(), _TRUNCATE );
      VariantClear( &vValue );
#ifndef UNICODE
      SAFEDELETEA( name );
#endif
      SysFreeString( bName );
      return true;
    }
    SysFreeString( bName );
    if ( pAttribMap ) pAttribMap->Release();
    pNodeMap->nextNode( &pAttribMap );
    VariantClear( &vValue );
  }


#ifndef UNICODE
  SAFEDELETEA( name );
#endif

  if ( pNodeMap ) pNodeMap->Release();

  return false;
}


CString CXMLNode::GetAttribute( TCHAR * szAttribute )
{
  MSXML2::IXMLDOMNamedNodeMap * pNodeMap = NULL;
  MSXML2::IXMLDOMNode * pAttribMap = NULL;

  pNode->get_attributes( &pNodeMap );
  pNodeMap->nextNode( &pAttribMap );

#ifndef UNICODE
	TS32 Size = (TS32)strlen(szAttribute);
	wchar_t *name = new wchar_t[Size + 2];
	name[Size + 1] = 0;
	if (!MultiByteToWideChar(CP_UTF8, 0, szAttribute, -1, name, Size + 1))
	{
		SAFEDELETEA(name);
		return _T("");
	}
#else
	wchar_t *name = szAttribute;
#endif

  while ( pAttribMap )
  {
    WCHAR * bName;
    VARIANT vValue;
    pAttribMap->get_nodeName( &bName );
    pAttribMap->get_nodeValue( &vValue );
    if ( wcscmp( bName, name ) == 0 )
    {
      if ( pAttribMap ) pAttribMap->Release();
      if ( pNodeMap ) pNodeMap->Release();
      CString v = vValue.bstrVal;
      VariantClear( &vValue );
#ifndef UNICODE
      SAFEDELETEA( name );
#endif
      SysFreeString( bName );
      return v;
    }
    SysFreeString( bName );
    if ( pAttribMap ) pAttribMap->Release();
    pNodeMap->nextNode( &pAttribMap );
    VariantClear( &vValue );
  }

#ifndef UNICODE
  SAFEDELETEA( name );
#endif
  if ( pNodeMap ) pNodeMap->Release();
  return _T( "" );
}


CString CXMLNode::GetAttributeAsString( TCHAR * szAttribute )
{
  MSXML2::IXMLDOMNamedNodeMap * pNodeMap = NULL;
  MSXML2::IXMLDOMNode * pAttribMap = NULL;

  pNode->get_attributes( &pNodeMap );
  pNodeMap->nextNode( &pAttribMap );

  while ( pAttribMap )
  {
    WCHAR * bName;
    VARIANT vValue;
    pAttribMap->get_nodeName( &bName );
    pAttribMap->get_nodeValue( &vValue );
    CString s = bName;
    SysFreeString( bName );
    if ( s == szAttribute )
    {
      if ( pAttribMap ) pAttribMap->Release();
      if ( pNodeMap ) pNodeMap->Release();
      CString v = vValue.bstrVal;
      VariantClear( &vValue );
      return v;
    }
    if ( pAttribMap ) pAttribMap->Release();
    pNodeMap->nextNode( &pAttribMap );
    VariantClear( &vValue );
  }

  if ( pNodeMap ) pNodeMap->Release();
  return _T( "" );
}

TBOOL CXMLNode::HasAttribute( TCHAR * szAttribute )
{
  MSXML2::IXMLDOMNamedNodeMap * pNodeMap = NULL;
  MSXML2::IXMLDOMNode * pAttribMap = NULL;

  pNode->get_attributes( &pNodeMap );
  pNodeMap->nextNode( &pAttribMap );

  while ( pAttribMap )
  {
    WCHAR * bName;
    VARIANT vValue;
    pAttribMap->get_nodeName( &bName );
    pAttribMap->get_nodeValue( &vValue );
    CString s = bName;
    SysFreeString( bName );
    if ( s == szAttribute )
    {
      if ( pAttribMap ) pAttribMap->Release();
      if ( pNodeMap ) pNodeMap->Release();
      VariantClear( &vValue );
      return true;
    }
    if ( pAttribMap ) pAttribMap->Release();
    pNodeMap->nextNode( &pAttribMap );
    VariantClear( &vValue );
  }

  if ( pNodeMap ) pNodeMap->Release();
  return false;
}

TS32 CXMLNode::IsValid()
{
  return pNode != NULL;
}

void CXMLNode::GetAttributeAsInteger( TCHAR * szAttribute, TS32 * pnValue )
{
  TCHAR s[ 20 ];
  ZeroMemory( s, 20 );
  GetAttribute( szAttribute, s, 20 );
  _stscanf_s( s, _T( "%d" ), pnValue );
}

void CXMLNode::GetAttributeAsFloat( TCHAR * szAttribute, TF32 * pfValue )
{
  TCHAR s[ 20 ];
  ZeroMemory( s, 20 );
  GetAttribute( szAttribute, s, 20 );
  _stscanf_s( s, _T( "%g" ), pfValue );
}

//void CXMLNode::FlushNode() 
//{
//	MSXML2::IXMLDOMNode * pChild=NULL;
//	pNode->get_firstChild(&pChild);
//	while (pChild) 
//	{
//		MSXML2::IXMLDOMNode * pOldChild=NULL;
//		MSXML2::IXMLDOMNode * pRemoveChild=pChild;
//		//pChild->get_nextSibling(&pChild);
//		MSXML2::IXMLDOMNode * pSibling = NULL;
//		pChild->get_nextSibling(&pSibling);
//		pChild->Release();
//		pChild = pSibling;
//
//		pNode->removeChild(pRemoveChild,&pOldChild);
//		if (pOldChild) pOldChild->Release();
//	}
//}

CXMLNode CXMLNode::AddChild( TCHAR * szNodeName, TBOOL PostEnter )
{
  IGNOREFREEERRORS( true );
  MSXML2::IXMLDOMNode * pNewNode = NULL;
  MSXML2::IXMLDOMNode * pRetVal = NULL;
  MSXML2::IXMLDOMNode * pDummy = NULL;

  TCHAR s[] = _T( "\r\n                              " );
  s[ nLevel * 2 + 2 ] = 0;

  pDoc->GetDoc()->createNode( _variant_t( (TS32)MSXML2::NODE_TEXT ), NULL, NULL, &pNewNode );
  pNewNode->put_nodeValue( _variant_t( s ) );
  pNode->appendChild( pNewNode, NULL );
  pNewNode->Release();

  {
    _bstr_t b = szNodeName;
    pDoc->GetDoc()->createNode( _variant_t( (TS32)MSXML2::NODE_ELEMENT ), b, NULL, &pNewNode );
    pNode->appendChild( pNewNode, &pRetVal );
    pNewNode->Release();
  }

  if ( PostEnter )
  {
    s[ nLevel * 2 ] = 0;
    pDoc->GetDoc()->createNode( _variant_t( (TS32)MSXML2::NODE_TEXT ), NULL, NULL, &pNewNode );
    pNewNode->put_nodeValue( _variant_t( s ) );
    pNode->appendChild( pNewNode, NULL );
    pNewNode->Release();
  }

  IGNOREFREEERRORS( false );
  return CXMLNode( pRetVal, pDoc, nLevel + 1 );
}

void CXMLNode::SetAttribute( TCHAR * szAttributeName, const TCHAR * szValue )
{
  IGNOREFREEERRORS( true );
  {
    VARIANT vURL;
    VariantInit( &vURL );
    vURL.vt = VT_BSTR;
    _bstr_t b = szValue;
    vURL.bstrVal = SysAllocString( b );
    //  vURL.intVal = 303;  

    MSXML2::IXMLDOMElement * pElement;
    pNode->QueryInterface( MSXML2::IID_IXMLDOMElement, (void**)&pElement );
    b = szAttributeName;
    HRESULT h = pElement->setAttribute( b, vURL );
    pElement->Release();

    SysFreeString( vURL.bstrVal );
  }
  IGNOREFREEERRORS( false );
}

void CXMLNode::SetAttributeFromInteger( TCHAR * szAttributeName, TS32 nValue )
{
  TCHAR s[ 64 ];
  _sntprintf_s( s, 64, _T( "%d" ), nValue );
  SetAttribute( szAttributeName, s );
}

void CXMLNode::SetAttributeFromFloat( TCHAR * szAttributeName, TF32 fValue )
{
  TCHAR s[ 64 ];
  _sntprintf_s( s, 64, _T( "%g" ), fValue );
  SetAttribute( szAttributeName, s );
}

void CXMLNode::SetText( const TCHAR * sz )
{
  IGNOREFREEERRORS( true );
  {
    _bstr_t bs;

#ifndef UNICODE
    if ( sz )
    {
      TS32 Size = strlen( sz );
      wchar_t *name = new wchar_t[ Size + 2 ];
      name[ Size + 1 ] = 0;
      if ( !MultiByteToWideChar( CP_UTF8, 0, sz, -1, name, Size + 1 ) )
      {
        SAFEDELETEA( name );
        return;
      }
      bs = name;
      delete[] name;
    }
#else
    bs = sz;
#endif
    pNode->put_text( bs );
  }
  IGNOREFREEERRORS( false );
}

void CXMLNode::SetText( CString &s )
{
  SetText( s.GetPointer() );
}

CString CXMLNode::SaveToString()
{
  BSTR bs;
  pNode->get_xml( &bs );
  CString s = CString( bs );
  SysFreeString( bs );

  return s;
}

void CXMLNode::SetInt( TS32 Int )
{
  TCHAR s[ 64 ];
  _sntprintf_s( s, 64, _T( "%d" ), Int );
  SetText( s );
}

void CXMLNode::SetFloat( TF32 Float )
{
  TCHAR s[ 64 ];
  _sntprintf_s( s, 64, _T( "%g" ), Float );
  SetText( s );
}

TBOOL CXMLNode::GetValue( TS32 &Int )
{
  TCHAR s[ 20 ];
  ZeroMemory( s, 20 );
  GetText( s, 20 );
  return _stscanf_s( s, _T( "%d" ), &Int ) == 1;
}

TBOOL CXMLNode::GetValue( TBOOL &Int )
{
  TCHAR s[ 20 ];
  ZeroMemory( s, 20 );
  GetText( s, 20 );
  TS32 x = 0;
  TS32 r = _stscanf_s( s, _T( "%d" ), &x );
  if ( r == 1 )
    Int = x != 0;
  return r == 1;
}

TBOOL CXMLNode::GetValue( TF32 &Float )
{
  TCHAR s[ 20 ];
  ZeroMemory( s, 20 );
  GetText( s, 20 );
  return _stscanf_s( s, _T( "%g" ), &Float ) == 1;
}

