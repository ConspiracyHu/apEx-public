#include "XMLDocument.h"

CXMLDocument::CXMLDocument( void )
{
  HRESULT res = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
  if ( res != S_OK && res != S_FALSE )
    LOG_ERR( "[XML] Error during CoInitializeEx" );

  pDoc = NULL;

  Allocate();
}

CXMLDocument::~CXMLDocument( void )
{
  Cleanup();
  CoUninitialize();
}

TBOOL CXMLDocument::LoadFromFile( TCHAR * szFileName )
{
  IGNOREFREEERRORS( true );

  if ( !pDoc )
    return false;

  VARIANT_BOOL vb = false;

  VARIANT vURL;
  VariantInit( &vURL );
  vURL.vt = VT_BSTR;
  vURL.bstrVal = _bstr_t( szFileName );

  HRESULT h = pDoc->load( vURL, &vb );

  IGNOREFREEERRORS( false );

  if ( FAILED( h ) || !vb )
  {
    LOG_ERR( "[XML] Failed to load document. Error code %d", h );

    MSXML2::IXMLDOMParseError *pError = NULL;
    BSTR sReason, sSource;
    long nLine = 0, nColumn = 0;

    pDoc->get_parseError( &pError );
    if ( pError )
    {
      pError->get_reason( &sReason );
      pError->get_srcText( &sSource );
      pError->get_line( &nLine );
      pError->get_linepos( &nColumn );
      LOG_ERR( "[XML] Load Failed at line %d column %d (%s)", nLine, nColumn, CString( (WCHAR*)sReason ).GetPointer() );
    }
    return false;
  }

  return true;
}

// throws HRESULT as a long on FAILED
#define HRCALL( a ) \
{ \
	HRESULT __hr; \
	__hr = (a); \
	if( FAILED( __hr ) ) \
	throw (long)__hr; \
}

// throws HRESULT as a long on FAILED or if !b is true
#define HRCALLRV( a, b ) \
{ \
	HRESULT __hr; \
	__hr = (a); \
	if( FAILED( __hr ) || !(b) ) \
	throw (long)__hr; \
}

TBOOL CXMLDocument::LoadFromString( CString s )
{
  IGNOREFREEERRORS( true );

  TBOOL retval = false;
  try
  {
    //// Some adjustments
    //HRCALL( pDoc->put_async(VARIANT_FALSE) );
    //HRCALL( pDoc->put_validateOnParse(VARIANT_FALSE) );
    //HRCALL( pDoc->put_resolveExternals(VARIANT_FALSE) );

    // Load the document
    VARIANT_BOOL vb;
    _bstr_t b = s.GetPointer();
    HRCALLRV( pDoc->loadXML( b, &vb ), vb );

    // Set the return flag to true
    retval = true;
  }
  catch ( long& /*hr*/ )
  {
    MSXML2::IXMLDOMParseError * error = NULL;
    pDoc->get_parseError( &error );
    BSTR b;
    error->get_reason( &b );

    LOG_ERR( "[xml] Xml Parsing Failed: %s", CString( b ).GetPointer() );
    SysFreeString( b );
    error->Release();

    //Cleanup();
  }

  IGNOREFREEERRORS( false );

  return retval;
}

CXMLNode CXMLDocument::GetDocumentNode()
{
  MSXML2::IXMLDOMNode * pRoot = NULL;
  pDoc->QueryInterface( MSXML2::IID_IXMLDOMNode, (void**)&pRoot );

  return CXMLNode( pRoot, this, 0 );
}

CString CXMLDocument::SaveToString()
{
  BSTR bs;
  pDoc->get_xml( &bs );
  CString s = CString( bs );

  SysFreeString( bs );

  return s;
}

TBOOL CXMLDocument::SaveToFile(TCHAR * sz)
{
	CString s = SaveToString();

	HANDLE h = CreateFile(sz, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
	if (h == INVALID_HANDLE_VALUE) return false;
	char * sz8 = new char[s.Length() * 3];
	s.WriteAsMultiByte(sz8, s.Length() * 3);
	DWORD b;
	WriteFile(h, sz8, (TS32)strlen(sz8), &b, NULL);
	CloseHandle(h);

	delete[] sz8;

	return true;
}

TBOOL CXMLDocument::Allocate()
{
  if ( pDoc ) return true;

  if ( CoCreateInstance( MSXML2::CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, MSXML2::IID_IXMLDOMDocument, (void**)&pDoc ) != S_OK )
  {
    LOG_ERR( "[XML] Error during CoCreateInstance!" );
    return false;
  }

  if ( !pDoc )
  {
    LOG_ERR( "[XML] Failed to create document object!" );
    return false;
  }

  pDoc->put_async( VARIANT_FALSE );
  pDoc->put_validateOnParse( VARIANT_FALSE );
  pDoc->put_resolveExternals( VARIANT_FALSE );

  return true;
}

TBOOL CXMLDocument::Cleanup()
{
  if ( pDoc )
  {
    pDoc->Release();
    pDoc = NULL;
  }

  CoFreeUnusedLibraries();
  return true;
}

CXMLNode CXMLDocument::CreateNode( VARIANT type, BSTR Name, TS32 Level )
{
  MSXML2::IXMLDOMNode * pNewNode = NULL;
  pDoc->createNode( _variant_t( (TS32)MSXML2::NODE_TEXT ), NULL, NULL, &pNewNode );
  return CXMLNode( pNewNode, this, Level );
}
