// DVDSpy.h : Interface to Girder via DVDSpy
// $Header$

// Allow various combinations of OLE and C strings as arguments.

extern void GirderEvent(BSTR event, BSTR pld);
extern void GirderEvent(LPCSTR event, BSTR pld);
extern void GirderEvent(LPCSTR event, LPCSTR pld);
