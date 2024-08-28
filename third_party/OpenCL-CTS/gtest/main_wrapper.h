#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#    define EXPORT __declspec(dllexport)
#else
#    define EXPORT __attribute__((visibility("default")))
#endif

EXPORT int main(int argc, const char *argv[]);

#ifdef __cplusplus
}
#endif

