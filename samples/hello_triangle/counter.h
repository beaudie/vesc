// C 3, shaders 2
#define COUNTER1 \
    int counter() { \
        int n = 0; \
        int i = 0; \
        bool s0 = false; \
        while(true) { \
            bool bar = s0; \
            if (!bar) { \
                bar = i < 3; \
                i = i + 1; \
            } \
            bool foo = !bar; \
            if (foo) { \
                break; \
            } \
            s0 = false; \
            n ++; \
        } \
        return n; \
    }

// All 3
#define COUNTER2 \
    int counter() { \
        int n = 0; \
        int i = 0; \
        while(true) { \
            bool bar = i < 3; \
            i = i + 1; \
            bool foo = !bar; \
            if (foo) { \
                break; \
            } \
            n ++; \
        } \
        return n; \
    }

// Vertex 3, others 4
#define COUNTER3 \
    int counter() { \
        int n = 0; \
        int i = 0; \
        bool s0 = true; \
        while(true) { \
            bool bar = s0; \
            if (!bar) { \
                bar = i < 3; \
                i = i + 1; \
            } \
            bool foo = !bar; \
            if (foo) { \
                break; \
            } \
            s0 = false; \
            n ++; \
        } \
        return n; \
    }

#define COUNTER4 \
    int counter() { \
        int n = 0; \
        int i = 0; \
        bool s0 = true; \
        while(true) { \
            bool bar = s0; \
            if (!bar) { \
                bar = i++ < 3; \
            } \
            bool foo = !bar; \
            if (foo) { \
                break; \
            } \
            s0 = false; \
            n ++; \
        } \
        return n; \
    }

#define COUNTER5 \
    int counter() { \
        int n = 0; \
        int i = 0; \
        bool s0 = true; \
        while(true) { \
            bool bar = s0 || (i++ < 3); \
            bool foo = !bar; \
            if (foo) { \
                break; \
            } \
            s0 = false; \
            n ++; \
        } \
        return n; \
    }

#define COUNTER6 \
    int counter() { \
        int n = 0; \
        int i = 0; \
        bool s0 = true; \
        while(true) { \
            if (!(s0 || (i++ < 3))) { \
                break; \
            } \
            s0 = false; \
            n ++; \
        } \
        return n; \
    }

#define COUNTER7 \
    int counter() { \
        int n = 0; \
        int i = 0; \
        bool s0 = true; \
        while(s0 || (i++ < 3)) { \
            s0 = false; \
            n ++; \
        } \
        return n; \
    }

#define STRINGIFYHELPER(arg) #arg
#define STRINGIFY(arg) STRINGIFYHELPER(arg)
#define COUNTER_STR STRINGIFY(COUNTER)
