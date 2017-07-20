#ifndef _NES_H_
#define _NES_H_

class NESController : Controller {
    unsigned char PadState;

    public:
        bool Init( void );
        bool Poll( void );

        bool Up( void );
        bool Down( void );
        bool Left( void );
        bool Right( void );

        bool Start( void );
        bool Select( void );

        bool A( void );
        bool B( void );

        virtual bool CPad( int* X, int* Y ) { };
        virtual bool CStick( int* X, int* Y ) { };        
};

#endif
