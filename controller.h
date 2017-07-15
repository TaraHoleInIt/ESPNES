#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

class Controller {
    public:
        virtual bool Init( void ) { };
        virtual void DeInit( void ) { };

        virtual bool Poll( void ) { return false; }

        virtual bool A( void ) { return false; }
        virtual bool B( void ) { return false; }
        virtual bool X( void ) { return false; }
        virtual bool Y( void ) { return false; }

        virtual bool Up( void ) { return false; }
        virtual bool Down( void ) { return false; }
        virtual bool Left( void ) { return false; }
        virtual bool Right( void ) { return false; }

        virtual bool Start( void ) { return false; }
        virtual bool Select( void ) { return false; }
};

#endif
