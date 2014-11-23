#ifndef TP_MSG_CRACK_H_INCLUDED
#define TP_MSG_CRACK_H_INCLUDED

#include <windows.h>
#include <string>
#include <map>

namespace tp
{
    class msg_crack
    {
    public:
        static msg_crack * instance()
        {
            static msg_crack mc;
            return &mc;
        }
        static const wchar_t* msgname(UINT msg, wchar_t* buffer, size_t buflen)
        {
            return msg_crack::instance()->crack(msg, buffer, buflen);
        }

        const wchar_t* crack(UINT message, wchar_t* buffer, size_t buflen)
        {
            if (message < WM_USER)
            {
                return m_mnmap[message];
            }
            else
            {
                // RegisterWindowMessage
                if (message >= 0xC000 && message <= 0xFFFF)
                {
                    // http://stackoverflow.com/questions/10780402/an-exported-aliases-symbol-doesnt-exist-in-pdb-file-registerclipboardformat-ha
                    // from the link above, RegisterWindowMessage and RegisterClipboardFormat share same RVA
                    // in other words, GetClipboardFormatName can be used to retrieve message name registered by RegisterWindowMessage
                    int len = ::GetClipboardFormatName(message, buffer, static_cast<int>(buflen));
                    buffer[len] = '\0';
                    return buffer;
                }
            }
            return NULL;
        }

    private:
        const wchar_t* m_mnmap[0x400];

        msg_crack()
        {
            construct_mnmap();
        }
        msg_crack(const msg_crack&);
        msg_crack& operator = (const msg_crack&);

        void construct_mnmap()
        {
#define TP_MC_ADD_MAP(x) m_mnmap[x] = L###x
            TP_MC_ADD_MAP(WM_CLOSE);
            TP_MC_ADD_MAP(WM_NULL);
            TP_MC_ADD_MAP(WM_CREATE);
            TP_MC_ADD_MAP(WM_DESTROY);
            TP_MC_ADD_MAP(WM_MOVE);
            TP_MC_ADD_MAP(WM_SIZE);
            TP_MC_ADD_MAP(WM_ACTIVATE);
            TP_MC_ADD_MAP(WM_SETFOCUS);
            TP_MC_ADD_MAP(WM_KILLFOCUS);
            TP_MC_ADD_MAP(WM_ENABLE);
            TP_MC_ADD_MAP(WM_SETREDRAW);
            TP_MC_ADD_MAP(WM_SETTEXT);
            TP_MC_ADD_MAP(WM_GETTEXT);
            TP_MC_ADD_MAP(WM_GETTEXTLENGTH);
            TP_MC_ADD_MAP(WM_PAINT);
            TP_MC_ADD_MAP(WM_CLOSE);
            TP_MC_ADD_MAP(WM_QUERYENDSESSION);
            TP_MC_ADD_MAP(WM_QUIT);
            TP_MC_ADD_MAP(WM_QUERYOPEN);
            TP_MC_ADD_MAP(WM_ERASEBKGND);
            TP_MC_ADD_MAP(WM_SYSCOLORCHANGE);
            TP_MC_ADD_MAP(WM_ENDSESSION);
            TP_MC_ADD_MAP(WM_SHOWWINDOW);
            TP_MC_ADD_MAP(WM_SETTINGCHANGE);
            TP_MC_ADD_MAP(WM_DEVMODECHANGE);
            TP_MC_ADD_MAP(WM_ACTIVATEAPP);
            TP_MC_ADD_MAP(WM_FONTCHANGE);
            TP_MC_ADD_MAP(WM_TIMECHANGE);
            TP_MC_ADD_MAP(WM_CANCELMODE);
            TP_MC_ADD_MAP(WM_SETCURSOR);
            TP_MC_ADD_MAP(WM_MOUSEACTIVATE);
            TP_MC_ADD_MAP(WM_CHILDACTIVATE);
            TP_MC_ADD_MAP(WM_QUEUESYNC);
            TP_MC_ADD_MAP(WM_GETMINMAXINFO);
            TP_MC_ADD_MAP(WM_PAINTICON);
            TP_MC_ADD_MAP(WM_ICONERASEBKGND);
            TP_MC_ADD_MAP(WM_NEXTDLGCTL);
            TP_MC_ADD_MAP(WM_SPOOLERSTATUS);
            TP_MC_ADD_MAP(WM_DRAWITEM);
            TP_MC_ADD_MAP(WM_MEASUREITEM);
            TP_MC_ADD_MAP(WM_DELETEITEM);
            TP_MC_ADD_MAP(WM_VKEYTOITEM);
            TP_MC_ADD_MAP(WM_CHARTOITEM);
            TP_MC_ADD_MAP(WM_SETFONT);
            TP_MC_ADD_MAP(WM_GETFONT);
            TP_MC_ADD_MAP(WM_SETHOTKEY);
            TP_MC_ADD_MAP(WM_GETHOTKEY);
            TP_MC_ADD_MAP(WM_QUERYDRAGICON);
            TP_MC_ADD_MAP(WM_COMPAREITEM);
            TP_MC_ADD_MAP(WM_GETOBJECT);
            TP_MC_ADD_MAP(WM_COMPACTING);
            TP_MC_ADD_MAP(WM_COMMNOTIFY);
            TP_MC_ADD_MAP(WM_WINDOWPOSCHANGING);
            TP_MC_ADD_MAP(WM_WINDOWPOSCHANGED);
            TP_MC_ADD_MAP(WM_POWER);
            TP_MC_ADD_MAP(WM_COPYDATA);
            TP_MC_ADD_MAP(WM_CANCELJOURNAL);
            TP_MC_ADD_MAP(WM_NOTIFY);
            TP_MC_ADD_MAP(WM_INPUTLANGCHANGEREQUEST);
            TP_MC_ADD_MAP(WM_INPUTLANGCHANGE);
            TP_MC_ADD_MAP(WM_TCARD);
            TP_MC_ADD_MAP(WM_HELP);
            TP_MC_ADD_MAP(WM_USERCHANGED);
            TP_MC_ADD_MAP(WM_NOTIFYFORMAT);
            TP_MC_ADD_MAP(WM_CONTEXTMENU);
            TP_MC_ADD_MAP(WM_STYLECHANGING);
            TP_MC_ADD_MAP(WM_STYLECHANGED);
            TP_MC_ADD_MAP(WM_DISPLAYCHANGE);
            TP_MC_ADD_MAP(WM_GETICON);
            TP_MC_ADD_MAP(WM_SETICON);
            TP_MC_ADD_MAP(WM_NCCREATE);
            TP_MC_ADD_MAP(WM_NCDESTROY);
            TP_MC_ADD_MAP(WM_NCCALCSIZE);
            TP_MC_ADD_MAP(WM_NCHITTEST);
            TP_MC_ADD_MAP(WM_NCPAINT);
            TP_MC_ADD_MAP(WM_NCACTIVATE);
            TP_MC_ADD_MAP(WM_GETDLGCODE);
            TP_MC_ADD_MAP(WM_SYNCPAINT);
            TP_MC_ADD_MAP(WM_NCMOUSEMOVE);
            TP_MC_ADD_MAP(WM_NCLBUTTONDOWN);
            TP_MC_ADD_MAP(WM_NCLBUTTONUP);
            TP_MC_ADD_MAP(WM_NCLBUTTONDBLCLK);
            TP_MC_ADD_MAP(WM_NCRBUTTONDOWN);
            TP_MC_ADD_MAP(WM_NCRBUTTONUP);
            TP_MC_ADD_MAP(WM_NCRBUTTONDBLCLK);
            TP_MC_ADD_MAP(WM_NCMBUTTONDOWN);
            TP_MC_ADD_MAP(WM_NCMBUTTONUP);
            TP_MC_ADD_MAP(WM_NCMBUTTONDBLCLK);
            TP_MC_ADD_MAP(WM_NCXBUTTONDOWN);
            TP_MC_ADD_MAP(WM_NCXBUTTONUP);
            TP_MC_ADD_MAP(WM_NCXBUTTONDBLCLK);
            TP_MC_ADD_MAP(WM_INPUT);
            TP_MC_ADD_MAP(WM_KEYDOWN);
            TP_MC_ADD_MAP(WM_KEYUP);
            TP_MC_ADD_MAP(WM_CHAR);
            TP_MC_ADD_MAP(WM_DEADCHAR);
            TP_MC_ADD_MAP(WM_SYSKEYDOWN);
            TP_MC_ADD_MAP(WM_SYSKEYUP);
            TP_MC_ADD_MAP(WM_SYSCHAR);
            TP_MC_ADD_MAP(WM_SYSDEADCHAR);
            TP_MC_ADD_MAP(WM_KEYLAST);
            TP_MC_ADD_MAP(WM_UNICHAR);
            TP_MC_ADD_MAP(WM_IME_STARTCOMPOSITION);
            TP_MC_ADD_MAP(WM_IME_ENDCOMPOSITION);
            TP_MC_ADD_MAP(WM_IME_COMPOSITION);
            TP_MC_ADD_MAP(WM_INITDIALOG);
            TP_MC_ADD_MAP(WM_COMMAND);
            TP_MC_ADD_MAP(WM_SYSCOMMAND);
            TP_MC_ADD_MAP(WM_TIMER);
            TP_MC_ADD_MAP(WM_HSCROLL);
            TP_MC_ADD_MAP(WM_VSCROLL);
            TP_MC_ADD_MAP(WM_INITMENU);
            TP_MC_ADD_MAP(WM_INITMENUPOPUP);
            TP_MC_ADD_MAP(WM_MENUSELECT);
            TP_MC_ADD_MAP(WM_MENUCHAR);
            TP_MC_ADD_MAP(WM_ENTERIDLE);
            TP_MC_ADD_MAP(WM_MENURBUTTONUP);
            TP_MC_ADD_MAP(WM_MENUDRAG);
            TP_MC_ADD_MAP(WM_MENUGETOBJECT);
            TP_MC_ADD_MAP(WM_UNINITMENUPOPUP);
            TP_MC_ADD_MAP(WM_MENUCOMMAND);
            TP_MC_ADD_MAP(WM_CHANGEUISTATE);
            TP_MC_ADD_MAP(WM_UPDATEUISTATE);
            TP_MC_ADD_MAP(WM_QUERYUISTATE);
            TP_MC_ADD_MAP(WM_CTLCOLORMSGBOX);
            TP_MC_ADD_MAP(WM_CTLCOLOREDIT);
            TP_MC_ADD_MAP(WM_CTLCOLORLISTBOX);
            TP_MC_ADD_MAP(WM_CTLCOLORBTN);
            TP_MC_ADD_MAP(WM_CTLCOLORDLG);
            TP_MC_ADD_MAP(WM_CTLCOLORSCROLLBAR);
            TP_MC_ADD_MAP(WM_CTLCOLORSTATIC);
            TP_MC_ADD_MAP(WM_MOUSEMOVE);
            TP_MC_ADD_MAP(WM_LBUTTONDOWN);
            TP_MC_ADD_MAP(WM_LBUTTONUP);
            TP_MC_ADD_MAP(WM_LBUTTONDBLCLK);
            TP_MC_ADD_MAP(WM_RBUTTONDOWN);
            TP_MC_ADD_MAP(WM_RBUTTONUP);
            TP_MC_ADD_MAP(WM_RBUTTONDBLCLK);
            TP_MC_ADD_MAP(WM_MBUTTONDOWN);
            TP_MC_ADD_MAP(WM_MBUTTONUP);
            TP_MC_ADD_MAP(WM_MBUTTONDBLCLK);
            TP_MC_ADD_MAP(WM_MOUSEWHEEL);
            TP_MC_ADD_MAP(WM_XBUTTONDOWN);
            TP_MC_ADD_MAP(WM_XBUTTONUP);
            TP_MC_ADD_MAP(WM_XBUTTONDBLCLK);
            TP_MC_ADD_MAP(WM_PARENTNOTIFY);
            TP_MC_ADD_MAP(WM_ENTERMENULOOP);
            TP_MC_ADD_MAP(WM_EXITMENULOOP);
            TP_MC_ADD_MAP(WM_NEXTMENU);
            TP_MC_ADD_MAP(WM_SIZING);
            TP_MC_ADD_MAP(WM_CAPTURECHANGED);
            TP_MC_ADD_MAP(WM_MOVING);
            TP_MC_ADD_MAP(WM_POWERBROADCAST);
            TP_MC_ADD_MAP(WM_DEVICECHANGE);
            TP_MC_ADD_MAP(WM_MDICREATE);
            TP_MC_ADD_MAP(WM_MDIDESTROY);
            TP_MC_ADD_MAP(WM_MDIACTIVATE);
            TP_MC_ADD_MAP(WM_MDIRESTORE);
            TP_MC_ADD_MAP(WM_MDINEXT);
            TP_MC_ADD_MAP(WM_MDIMAXIMIZE);
            TP_MC_ADD_MAP(WM_MDITILE);
            TP_MC_ADD_MAP(WM_MDICASCADE);
            TP_MC_ADD_MAP(WM_MDIICONARRANGE);
            TP_MC_ADD_MAP(WM_MDIGETACTIVE);
            TP_MC_ADD_MAP(WM_MDISETMENU);
            TP_MC_ADD_MAP(WM_ENTERSIZEMOVE);
            TP_MC_ADD_MAP(WM_EXITSIZEMOVE);
            TP_MC_ADD_MAP(WM_DROPFILES);
            TP_MC_ADD_MAP(WM_MDIREFRESHMENU);
            TP_MC_ADD_MAP(WM_IME_SETCONTEXT);
            TP_MC_ADD_MAP(WM_IME_NOTIFY);
            TP_MC_ADD_MAP(WM_IME_CONTROL);
            TP_MC_ADD_MAP(WM_IME_COMPOSITIONFULL);
            TP_MC_ADD_MAP(WM_IME_SELECT);
            TP_MC_ADD_MAP(WM_IME_CHAR);
            TP_MC_ADD_MAP(WM_IME_REQUEST);
            TP_MC_ADD_MAP(WM_IME_KEYDOWN);
            TP_MC_ADD_MAP(WM_IME_KEYUP);
            TP_MC_ADD_MAP(WM_NCMOUSEHOVER);
            TP_MC_ADD_MAP(WM_MOUSEHOVER);
            TP_MC_ADD_MAP(WM_NCMOUSELEAVE);
            TP_MC_ADD_MAP(WM_MOUSELEAVE);
            TP_MC_ADD_MAP(WM_WTSSESSION_CHANGE);
            TP_MC_ADD_MAP(WM_TABLET_FIRST);
            TP_MC_ADD_MAP(WM_TABLET_LAST);
            TP_MC_ADD_MAP(WM_CUT);
            TP_MC_ADD_MAP(WM_COPY);
            TP_MC_ADD_MAP(WM_PASTE);
            TP_MC_ADD_MAP(WM_CLEAR);
            TP_MC_ADD_MAP(WM_UNDO);
            TP_MC_ADD_MAP(WM_RENDERFORMAT);
            TP_MC_ADD_MAP(WM_RENDERALLFORMATS);
            TP_MC_ADD_MAP(WM_DESTROYCLIPBOARD);
            TP_MC_ADD_MAP(WM_DRAWCLIPBOARD);
            TP_MC_ADD_MAP(WM_PAINTCLIPBOARD);
            TP_MC_ADD_MAP(WM_VSCROLLCLIPBOARD);
            TP_MC_ADD_MAP(WM_SIZECLIPBOARD);
            TP_MC_ADD_MAP(WM_ASKCBFORMATNAME);
            TP_MC_ADD_MAP(WM_CHANGECBCHAIN);
            TP_MC_ADD_MAP(WM_HSCROLLCLIPBOARD);
            TP_MC_ADD_MAP(WM_QUERYNEWPALETTE);
            TP_MC_ADD_MAP(WM_PALETTEISCHANGING);
            TP_MC_ADD_MAP(WM_PALETTECHANGED);
            TP_MC_ADD_MAP(WM_HOTKEY);
            TP_MC_ADD_MAP(WM_PRINT);
            TP_MC_ADD_MAP(WM_PRINTCLIENT);
            TP_MC_ADD_MAP(WM_APPCOMMAND);
            TP_MC_ADD_MAP(WM_THEMECHANGED);
            TP_MC_ADD_MAP(WM_HANDHELDFIRST);
            TP_MC_ADD_MAP(WM_HANDHELDLAST);
            TP_MC_ADD_MAP(WM_AFXFIRST);
            TP_MC_ADD_MAP(WM_AFXLAST);
            TP_MC_ADD_MAP(WM_PENWINFIRST);
            TP_MC_ADD_MAP(WM_PENWINLAST);
#undef  TP_MC_ADD_MAP
        }
    };
}


#endif