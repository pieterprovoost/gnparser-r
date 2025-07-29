#include <Rcpp.h>
#include <dlfcn.h>
#include <string>

using namespace Rcpp;

typedef char* (*ParseToStringFunc)(const char*, const char*, const char*, int, int);
typedef void (*FreeMemoryFunc)(char*);

static void* lib_handle = nullptr;
static ParseToStringFunc parse_func = nullptr;
static FreeMemoryFunc free_func = nullptr;

bool load_library() {
    if (lib_handle != nullptr) return true;
    
    std::string lib_name;
    #ifdef __APPLE__
        lib_name = "libgnparser.dylib";
    #elif __linux__
        lib_name = "libgnparser.so";
    #elif _WIN32
        lib_name = "libgnparser.dll";
    #endif
    
    Function find_package("find.package");
    SEXP pkg_result = find_package("gnparser");
    std::string lib_path;
    if (TYPEOF(pkg_result) == STRSXP && LENGTH(pkg_result) > 0) {
        std::string pkg_path = CHAR(STRING_ELT(pkg_result, 0));
        lib_path = pkg_path + "/gnparser/" + lib_name;
    } else {
        Rf_error("Could not find gnparser package directory.");
        return false;
    }
    
    lib_handle = dlopen(lib_path.c_str(), RTLD_LAZY);
    if (!lib_handle) {
        Rf_error("Cannot load library: %s", dlerror());
        return false;
    }
    
    parse_func = (ParseToStringFunc) dlsym(lib_handle, "ParseToString");
    if (!parse_func) {
        dlclose(lib_handle);
        lib_handle = nullptr;
        Rf_error("Cannot find ParseToString function");
        return false;
    }
    
    free_func = (FreeMemoryFunc) dlsym(lib_handle, "FreeMemory");
    if (!free_func) {
        dlclose(lib_handle);
        lib_handle = nullptr;
        Rf_error("Cannot find FreeMemory function");
        return false;
    }
    
    return true;
}

// [[Rcpp::export]]
CharacterVector parse_names_internal(CharacterVector names, String format, Nullable<String> code, bool details, bool diaereses) {
    
    if (!load_library()) {
        stop("Failed to load gnparser library");
    }
    
    int n = names.size();
    CharacterVector results(n);
    
    std::string code_str = "";
    const char* code_ptr = nullptr;
    if (code.isNotNull()) {
        code_str = as<std::string>(code);
        code_ptr = code_str.c_str();
    }
    
    for (int i = 0; i < n; i++) {
        if (CharacterVector::is_na(names[i])) {
            results[i] = NA_STRING;
            continue;
        }
        
        std::string name_str = as<std::string>(names[i]);
        const char* format_cstr = format.get_cstring();
        
        char* result = parse_func(
            name_str.c_str(),
            format_cstr,
            code_ptr,
            details ? 1 : 0,
            diaereses ? 1 : 0
        );
        
        if (result) {
            results[i] = std::string(result);
            free_func(result);
        } else {
            results[i] = NA_STRING;
        }
    }
    
    return results;
}

void cleanup_library() {
    if (lib_handle) {
        dlclose(lib_handle);
        lib_handle = nullptr;
        parse_func = nullptr;
        free_func = nullptr;
    }
}

extern "C" void R_unload_gnparser(DllInfo* info) {
    cleanup_library();
}