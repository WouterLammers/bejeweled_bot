// This is not industry-strength code! This is the result of a hack session.
// Use this code at your own risk :)
// This code depends on psapi.lib and shlwapi.lib. If your VS version does
// not have these you might need to download the windows SDK
#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <ole2.h>
//#include <commctrl.h>
#include <psapi.h>
#include <shlwapi.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <deque>
#include <algorithm>
#include <utility>
#include <tuple>
#include <map>

using namespace ::std;

HINSTANCE g_hinst;                          /* This application's HINSTANCE */
HWND g_hwndChild;                           /* Optional child window */
HWND hwnd;
HWND twnd;
POINT offset;


HWND desktop;
HDC hdcDT;
HDC memDC = 0;

enum COLOUR { RED, GREEN, BLUE, PURPLE, YELLOW, ORANGE, WHITE, UNKNOWN, NONE };
enum CRANGE { LOW, MIDDLE, HIGH };

typedef pair<pair<int,int>,pair<int,int> > MOVE;

vector<MOVE> moves;

deque<wstring> msgs;

void out(const wchar_t* msg) 
{
   msgs.push_front(wstring(msg));
   InvalidateRect( hwnd, 0, TRUE );
}

void out(COLORREF c) 
{
   wstringstream ws;
   ws << "(" << GetRValue(c) << ", " << GetGValue(c) << ", " << GetBValue(c) << ")";
   out(ws.str().c_str());
}

void out(MOVE move)
{
   wstringstream ws;
   ws << "(" << move.first.first << "," << move.first.second << ") --> (" << move.second.first << "," << move.second.second << ")";
   out(ws.str().c_str());
}

struct Board
{
   vector< vector<COLOUR> > b;

   Board() : b(8, vector<COLOUR>(8, NONE))
   {
   }

   COLOUR operator() (int x, int y) 
   {
      if( x < 0 || x > 7) return NONE;
      if( y < 0 || y > 7 ) return NONE;

      return b[x][y];
   }
};
Board b;

COLORREF toCRef(const COLOUR c)
{
   switch(c)
   {
   case RED: return RGB(255,0,0);
   case GREEN: return RGB(0,255,0);
   case BLUE: return RGB(0,0,255);
   case PURPLE: return RGB(255,0,255);
   case YELLOW: return RGB(255,255,0);
   case ORANGE: return RGB(255,100,100);
   case WHITE: return RGB(255,255,255);
   default:
   case UNKNOWN: return RGB(0,0,0);
   }
}

vector<MOVE> getMoves(Board b)
{
   vector<MOVE> moves;
   // horizontal
   for( int i=7; i>=0; --i )
   {
      for( int j=7; j>=0; --j )
      {
         if( b(j,i) == UNKNOWN ) continue;

         if( b(j,i) == b(j-1,i) ) 
         {
            if(b(j,i) == b(j-3,i) )
               moves.push_back( make_pair(make_pair(j-3,i), make_pair(j-2,i)) );

            if( b(j,i) == b(j+2,i) )
               moves.push_back( make_pair(make_pair(j+2,i), make_pair(j+1,i)) );

            if( b(j+1,i-1) == b(j,i) ) 
               moves.push_back( make_pair(make_pair(j+1,i-1), make_pair(j+1,i)) );
            if( b(j+1,i+1) == b(j,i) ) 
               moves.push_back( make_pair(make_pair(j+1,i+1), make_pair(j+1,i)) );
            if( b(j-2,i-1) == b(j,i) ) 
               moves.push_back( make_pair(make_pair(j-2,i-1), make_pair(j-2,i)) );
            if( b(j-2,i+1) == b(j,i) ) 
               moves.push_back( make_pair(make_pair(j-2,i+1), make_pair(j-2,i)) );
         }

         // triplet
         if( b(j,i) == b(j-2,i) ) 
         {
            if( b(j-1, i+1) == b(j,i) )
               moves.push_back( make_pair(make_pair(j-1,i+1), make_pair(j-1,i)) );

            if( b(j-1, i-1) == b(j,i) )
               moves.push_back( make_pair(make_pair(j-1,i-1), make_pair(j-1,i)) );
         }

      }
   }

   // vertical
   for( int i=7; i>=0; --i )
   {
      for( int j=7; j>=0; --j )
      {
         if( b(i,j) == UNKNOWN ) continue;

         if( b(i,j) == b(i,j-1) ) 
         {
            if( b(i,j) == b(i,j-3) )
              moves.push_back( make_pair(make_pair(i, j-3), make_pair(i, j-2)) );

            if( b(i,j) == b(i,j+2) )
              moves.push_back( make_pair(make_pair(i,j+2), make_pair(i,j+1)) );

            if( b(i+1,j+1) == b(i,j) )
              moves.push_back( make_pair(make_pair(i+1,j+1), make_pair(i,j+1)) );
            if( b(i-1,j+1) == b(i,j) )
              moves.push_back( make_pair(make_pair(i-1,j+1), make_pair(i,j+1)) );
            if( b(i-1,j-2) == b(i,j) )
              moves.push_back( make_pair(make_pair(i-1,j-2), make_pair(i,j-2)) );
            if( b(i+1,j-2) == b(i,j) )
              moves.push_back( make_pair(make_pair(i+1,j-2), make_pair(i,j-2)) );
         }

         if( b(i,j) == b(i,j-2) )
         {
            if( b(i-1,j-1) == b(i,j) ) 
              moves.push_back( make_pair(make_pair(i-1,j-1), make_pair(i,j-1)) );

            if( b(i+1,j-1) == b(i,j) ) 
              moves.push_back( make_pair(make_pair(i+1,j-1), make_pair(i,j-1)) );
         }
      }
   }

   return moves;
}


wchar_t* tostring(COLOUR c)
{
   switch(c) 
   {
   case RED : return L"RED";
   case GREEN : return L"GREEN";
   case BLUE : return L"BLUE";
   case PURPLE : return L"PURPLE";
   case YELLOW : return L"YELLOW";
   case ORANGE : return L"ORANGE";
   case WHITE : return L"WHITE";
   case UNKNOWN : return L"UNKNOWN";
   default:
   case NONE : return L"NONE";
   }
}

void out(Board b)
{
   for( int i=7; i>=0; --i ) 
   {
      wstringstream ws;
      for( int j=0; j<8; ++j )
      {
         ws << tostring(b(j,i)) << ", ";
      }
      out(ws.str().c_str());
   }
   out(L"----");
}

POINT lp2p( LPARAM lParam )
{
   POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
   return p;
}

//POINT toClient( POINT p )
//{
   //::ClientToScreen( hnd, &p );
   //::ScreenToClient( fvWnd, &p );
   //return p;
//}

//POINT toClient( int x, int y )
//{
   //POINT p = { x, y };
   //return toClient( p );
//}

void lmdown( POINT p )
{
//   POINT p = toClient(pp);
   ::PostMessage( twnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM( p.x, p.y ) ); 
}

void lmup( POINT p )
{
   //POINT p = toClient(pp);
   ::PostMessage( twnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM( p.x, p.y ) ); 
}

void click( POINT p )
{
   lmdown( p );
   lmup( p );
}

COLORREF getArea(POINT p)
{
   const int R = 10;
   const int RSQ = R*R;

   int r=0,g=0,b=0;
   for( int x = 0; x<R; ++x ) 
   {
      for( int y = 0; y<R; ++y ) 
      {
         COLORREF c = ::GetPixel(memDC, p.x+x, p.y+y);
         if( c == CLR_INVALID ) out(L"CLR_INVALID");
         r += GetRValue(c);
         g += GetGValue(c);
         b += GetBValue(c);
      }
   }

   return RGB(r/RSQ, g/RSQ, b/RSQ);
}

CRANGE determineRange(const int c)
{
   const int THRESHOLD_HI = 190;
   const int THRESHOLD_LO = 255-THRESHOLD_HI;

   if( c > THRESHOLD_HI ) return HIGH;
   if( c < THRESHOLD_LO ) return LOW;
   return MIDDLE;
}

COLOUR getColour(CRANGE r, CRANGE g, CRANGE b)
{
   typedef tuple<CRANGE, CRANGE, CRANGE> Coltup;
   static map<tuple<CRANGE, CRANGE, CRANGE>, COLOUR> colourMap;

   if( colourMap.empty() ) 
   {
      colourMap[Coltup(HIGH, LOW, LOW)] = RED;
      colourMap[Coltup(HIGH, LOW, MIDDLE)] = RED;
      colourMap[Coltup(HIGH, MIDDLE, LOW)] = RED;
      //colourMap[Coltup(HIGH, MIDDLE, MIDDLE)] = RED;

      colourMap[Coltup(LOW, HIGH, LOW)] = GREEN;
      colourMap[Coltup(MIDDLE, HIGH, LOW)] = GREEN;
      colourMap[Coltup(LOW, HIGH, MIDDLE)] = GREEN;
      colourMap[Coltup(MIDDLE, HIGH, MIDDLE)] = GREEN;

      colourMap[Coltup(LOW, LOW, HIGH)] = BLUE;
      colourMap[Coltup(LOW, MIDDLE, HIGH)] = BLUE;
      colourMap[Coltup(MIDDLE, LOW, HIGH)] = BLUE;
      colourMap[Coltup(MIDDLE, MIDDLE, HIGH)] = BLUE;

      colourMap[Coltup(HIGH, LOW, HIGH)] = PURPLE;

      colourMap[Coltup(HIGH, HIGH, LOW)] = YELLOW;

      colourMap[Coltup(HIGH, MIDDLE, MIDDLE)] = ORANGE;

      colourMap[Coltup(HIGH, HIGH, HIGH)] = WHITE;
   }

   auto iter = colourMap.find(Coltup(r,g,b));
   if( iter != colourMap.end() )
      return iter->second;
   else
      return UNKNOWN;
}

COLOUR determineColour(const COLORREF c)
{
   //out(c);
   CRANGE redR = determineRange(GetRValue(c));
   CRANGE greenR = determineRange(GetGValue(c));
   CRANGE blueR = determineRange(GetBValue(c));

   return getColour(redR, greenR, blueR);
}

/*
 *  OnSize
 *      If we have an inner child, resize it to fit.
 */
void
OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    if (g_hwndChild) {
        MoveWindow(g_hwndChild, 0, 0, cx, cy, TRUE);
    }
}

/*
 *  OnCreate
 *      Applications will typically override this and maybe even
 *      create a child window.
 */
BOOL
OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
    return TRUE;
}

/*
 *  OnDestroy
 *      Post a quit message because our application is over when the
 *      user closes this window.
 */
void
OnDestroy(HWND hwnd)
{
    PostQuitMessage(0);
}

void drawArrow(HDC hdc, const POINT p1, const POINT p2)
{
   const int SIZE = 6;

   POINT r1 = {240 + 20 + (p1.x*40), 20 + (p1.y*40)};
   POINT r2 = {240 + 20 + (p2.x*40), 20 + (p2.y*40)};

   if(p1.x != p2.x) 
   {
      // horizontal
      ::Rectangle(hdc, r1.x, r1.y-(SIZE/2), r2.x, r2.y+(SIZE/2));
      POINT rs[] = {{r2.x,r2.y-SIZE}, {r2.x+((r1.x<r2.x)?SIZE:-SIZE)*2,r2.y}, {r2.x,r2.y+SIZE}};
      ::Polygon(hdc, rs, 3);
   } 
   else
   {
      // vertical
      ::Rectangle(hdc, r1.x-(SIZE/2), r1.y, r2.x+(SIZE/2), r2.y);
      POINT rs[] = {{r2.x-SIZE,r2.y}, {r2.x+SIZE,r2.y}, {r2.x,r2.y+((r1.y<r2.y)?SIZE:-SIZE)*2}};
      ::Polygon(hdc, rs, 3);
   }

   ::Ellipse(hdc, r1.x-SIZE-4, r1.y-SIZE-4, r1.x+SIZE+4, r1.y+SIZE+4);
}
/*
 *  PaintContent
 *      Interesting things will be painted here eventually.
 */
void
PaintContent(HWND hwnd, PAINTSTRUCT *pps)
{
   ::SetBkMode(pps->hdc, TRANSPARENT);
   ::SelectObject(pps->hdc,GetStockObject(NULL_BRUSH));
   ::SelectObject(pps->hdc,GetStockObject(DC_PEN));

   if(memDC != 0)
   {
      BitBlt(pps->hdc, 240, 0, 320, 320, memDC, 0, 0, SRCCOPY);
   
      for( int i=0; i<8; ++i ) 
      {
	      for( int j=0; j<8; ++j ) 
   	   {
            ::SetDCPenColor(pps->hdc, toCRef(b(i,j)));
            for( int q=0; q<4; ++q )
            {
		         ::Rectangle(pps->hdc, 240+(i*40)+q, (j*40)+q, 280+(i*40)-q, 40+(j*40)-q);
            }
            if(b(i,j) == UNKNOWN)
            {
               POINT ps[] = {{240+(i*40), (j*40)},
                             {280+(i*40), 40+(j*40)}, 
                             {240+(i*40), (j*40)+1},
                             {280+(i*40)-1, 40+(j*40)},
                             {240+(i*40), (j*40)+2},
                             {280+(i*40)-2, 40+(j*40)},
                             {240+(i*40), (j*40)+3},
                             {280+(i*40)-3, 40+(j*40)},
                             {240+(i*40), (j*40)+4},
                             {280+(i*40)-4, 40+(j*40)},
                            };
               ::Polyline(pps->hdc, ps, 10);
            }
         }
      }
   }

   ::SelectObject(pps->hdc,GetStockObject(DC_BRUSH));
   ::SetDCPenColor(pps->hdc, RGB(0,0,0));
   for_each(moves.begin(), moves.end(), [=](const MOVE& m) 
   {
      ::SetDCBrushColor(pps->hdc, toCRef(b(m.first.first,m.first.second)));
      POINT p1 = {m.first.first,m.first.second};
      POINT p2 = {m.second.first,m.second.second};
      drawArrow(pps->hdc, p1, p2);
   });

   ::SetTextColor(pps->hdc, RGB(255,255,255));
   {
      int i = 0;
      for_each(msgs.begin(), msgs.end(), [=, &i] (const wstring& ws) {
         if( i<37 ) 
         {
            ::TextOut( pps->hdc, 9, 9 + (i)*20, ws.c_str(), ws.size() );
            ::TextOut( pps->hdc, 9, 11 + (i)*20, ws.c_str(), ws.size() );
            ::TextOut( pps->hdc, 9, 10 + (i)*20, ws.c_str(), ws.size() );
            ::TextOut( pps->hdc, 10, 9 + (i)*20, ws.c_str(), ws.size() );
            ::TextOut( pps->hdc, 11, 9 + (i)*20, ws.c_str(), ws.size() );
            ::TextOut( pps->hdc, 11, 11 + (i)*20, ws.c_str(), ws.size() );
            i++;
         }
      });
   }

   ::SetTextColor(pps->hdc, RGB(0,0,0));
   {
      int i = 0;
      for_each(msgs.begin(), msgs.end(), [=, &i] (const wstring& ws) {
         if( i<37 ) 
         {
            ::TextOut( pps->hdc, 10, 10 + (i)*20, ws.c_str(), ws.size() );
            i++;
         }
      });
   }
}

/*
 *  OnPaint
 *      Paint the content as part of the paint cycle.
 */
void
OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);
    PaintContent(hwnd, &ps);
    EndPaint(hwnd, &ps);
}

/*
 *  OnPrintClient
 *      Paint the content as requested by USER.
 */
void
OnPrintClient(HWND hwnd, HDC hdc)
{
    PAINTSTRUCT ps;
    ps.hdc = hdc;
    GetClientRect(hwnd, &ps.rcPaint);
    PaintContent(hwnd, &ps);

}

bool timerRunning = false;

// MyTimerProc is an application-defined callback function that 
// processes WM_TIMER messages. 
 
VOID CALLBACK MyTimerProc( 
    HWND hwnd,        // handle to window for timer messages 
    UINT message,     // WM_TIMER message 
    UINT idTimer,     // timer identifier 
    DWORD dwTime)     // current system time 
{ 
      BitBlt(memDC, 0, 0, 320, 320, ::GetDC(twnd), offset.x, offset.y, SRCCOPY);

      for(int i=0; i<8; ++i)
      {
         for(int j=0; j<8; ++j)
         {
            POINT pp = {15+(j*40), 15+(i*40)};
            b.b[j][i] = determineColour(getArea(pp));
         }
      }

      moves.swap(getMoves(b));

      for_each(moves.begin(), moves.end(), [](const MOVE& m) 
      {
         POINT pc1 = {offset.x +20 + (m.first.first*40), offset.y + 20 + (m.first.second*40)};
         POINT pc2 = {offset.x +20 + (m.second.first*40), offset.y + 20 + (m.second.second*40)};
         click(pc1);
         click(pc2);
      });

      ::InvalidateRect( hwnd, 0, TRUE );
}

int period = 100;
void flipTimer()
{
   if( !timerRunning )
   {
      UINT uResult = SetTimer(hwnd,      // handle to main window 
       123,            // timer identifier 
       period,                    // 10-second interval 
       (TIMERPROC) MyTimerProc); // timer callback 
   }
   else
   {
      ::KillTimer(hwnd, 123);
   }
   timerRunning = !timerRunning;

   wstringstream ws;
   ws << "Period: " << period << " timerenabled: " << timerRunning;
   out(ws.str().c_str());
}

LRESULT onKeyDown(WPARAM wParam)
{
   POINT p;
   if( !::GetCursorPos( &p ) ) ::MessageBox( hwnd, L"Err getcursorpos", L"Err", 0 );

   if( wParam == 81 ) // Q
	{
      twnd = WindowFromPoint( p );
		if( twnd == NULL )
		   out(L"windowfrompoint null");
		else
   	{
         DWORD pid;
         ::GetWindowThreadProcessId( twnd, &pid );

         HANDLE ph = ::OpenProcess( PROCESS_QUERY_INFORMATION, FALSE, pid );
         if( ph )
         {
            TCHAR buf[1024];
            ::GetProcessImageFileName( ph, buf, 1024 );
            out(::PathFindFileName( buf ));
            if( ::GetWindowText( twnd, buf, 1024 ) != 0 )
            {
               out(buf);
            }
         }

//         memDC = CreateCompatibleDC ( ::GetDC(twnd) );
//         HBITMAP memBM = CreateCompatibleBitmap ( ::GetDC(twnd), 320, 320 );
         memDC = CreateCompatibleDC ( hdcDT );
         HBITMAP memBM = CreateCompatibleBitmap ( hdcDT, 320, 320 );
         SelectObject ( memDC, memBM );

         offset.x = p.x; offset.y = p.y;
         ::ScreenToClient(twnd, &offset);

         int unknowns = 100;
         POINT startoffset = {offset.x, offset.y};
         POINT bestoffset = {offset.x, offset.y};

         for( int x = 0; x < 6; ++x )
         {
            if( unknowns == 0 ) break;

            for( int y = 0; y < 6; ++y )
            {
               if( unknowns == 0 ) break;

               offset.x = startoffset.x+x;
               offset.y = startoffset.y+y;
               BitBlt(memDC, 0, 0, 320, 320, ::GetDC(twnd), offset.x, offset.y, SRCCOPY);

               for(int i=0; i<8; ++i)
               {
                  for(int j=0; j<8; ++j)
                  {
                     POINT pp = {15+(j*40), 15+(i*40)};
                     b.b[j][i] = determineColour(getArea(pp));
                  }
               }

               vector<MOVE> moves = getMoves(b);
               int countmistakes = 0;
               for(int i=0; i<8; ++i)
               {
                  for(int j=0; j<8; ++j)
                  {
                     if( b(i,j) == UNKNOWN ) ++countmistakes;
                  }
               }

               if( countmistakes < unknowns ) 
               {
                  bestoffset.x = offset.x;
                  bestoffset.y = offset.y;
                  unknowns = countmistakes;

                  wstringstream ws;
                  ws << x << ", " << y << "  ->  " << unknowns;
                  out(ws.str().c_str());
               }
            }
         }

         offset.x = bestoffset.x;
         offset.y = bestoffset.y;
      }
   }

   if( wParam == 65 ) //a
   {
      flipTimer();
   }

   if( wParam == 66 ) // b 
   {
//         memDC = CreateCompatibleDC ( hdcDT );
//         HBITMAP memBM = CreateCompatibleBitmap ( hdcDT, 320, 320 );
//         SelectObject ( memDC, memBM );
//      BitBlt(memDC, 0, 0, 320, 320, hdcDT, p.x, p.y, SRCCOPY);
      BitBlt(memDC, 0, 0, 320, 320, ::GetDC(twnd), offset.x, offset.y, SRCCOPY);

      for(int i=0; i<8; ++i)
      {
         for(int j=0; j<8; ++j)
         {
//            POINT pp = {p.x+15+(j*40), p.y+15+(i*40)};
            POINT pp = {15+(j*40), 15+(i*40)};
            b.b[j][i] = determineColour(getArea(pp));
            //if( i==2 && j == 0 ) {
            //   out(getArea(pp));
            //}
            //out(tostring(b.b[j][i]));
         }
      }

      //out(b);
      moves.swap(getMoves(b));

//      MOVE m = moves.front();
      for_each(moves.begin(), moves.end(), [](const MOVE& m) 
      {
         POINT pc1 = {offset.x +20 + (m.first.first*40), offset.y + 20 + (m.first.second*40)};
         POINT pc2 = {offset.x +20 + (m.second.first*40), offset.y + 20 + (m.second.second*40)};
//         out(m); out(L"--");
//         wstringstream ws;
//         ws << pc1.x << ", " << pc1.y << "  -->  " << pc2.x << ", " << pc2.y;
//         out(ws.str().c_str());
         click(pc1);
         click(pc2);
      });

      ::InvalidateRect( hwnd, 0, TRUE );
   }

   if( wParam == 67 ) // c 
   {
      period -= 100;
      wstringstream ws;
      ws << "Period: " << period << " timerenabled: " << timerRunning;
      out(ws.str().c_str());
   }
   if( wParam == 68 ) // d
   {
      period +=100;
      wstringstream ws;
      ws << "Period: " << period << " timerenabled: " << timerRunning;
      out(ws.str().c_str());
   }

   return 0;
	//			POINT p;
//				if( !::GetCursorPos( &p ) ) ::MessageBox( hWnd, L"Err", L"Err", 0 );
				//std::wostringstream os;
				//os << L"Pos: " << p.x << L", " << p.y << L"   " << wParam;
				//SetWindowText( hWnd, os.str().c_str() );
}

/*
 *  Window procedure
 */
LRESULT CALLBACK
WndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uiMsg) {

    HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
    HANDLE_MSG(hwnd, WM_SIZE, OnSize);
    HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
    HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
    case WM_PRINTCLIENT: OnPrintClient(hwnd, (HDC)wParam); return 0;
    case WM_KEYDOWN: return onKeyDown(wParam);
    }

    return DefWindowProc(hwnd, uiMsg, wParam, lParam);
}

BOOL
InitApp(void)
{
    WNDCLASS wc;

    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_hinst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = TEXT("Scratch");

    if (!RegisterClass(&wc)) return FALSE;

    //InitCommonControlsEx();               /* In case we use a common control */

    return TRUE;
}

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hinstPrev,
                   LPSTR lpCmdLine, int nShowCmd)
{
    MSG msg;

    g_hinst = hinst;

    if (!InitApp()) return 0;

    if (SUCCEEDED(CoInitialize(NULL))) {/* In case we use COM */

        hwnd = CreateWindow(
            TEXT("Scratch"),                      /* Class Name */
            TEXT("Scratch"),                      /* Title */
            WS_OVERLAPPEDWINDOW,            /* Style */
            0, 100,   /* Position */
            600, 800,   /* Size */
            NULL,                           /* Parent */
            NULL,                           /* No menu */
            hinst,                          /* Instance */
            0);                             /* No special parameters */

        desktop =::GetDesktopWindow();
        hdcDT = ::GetDC(desktop);

        ShowWindow(hwnd, nShowCmd);

        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        CoUninitialize();
    }

    return 0;
}
