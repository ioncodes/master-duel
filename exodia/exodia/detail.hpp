#pragma once

#include <iostream>

namespace detail
{
    enum class offsets : uintptr_t
    {
        card_content_get_name = 0x7be690,
        card_content_get_level = 0x7be120,
        card_content_get_desc = 0x7bd900,
        gc_register_thread = 0x20f000
    };

    namespace builtins
    {
        struct string_t
        {
            string_t* self;
            void* monitor;
            int32_t length;
            wchar_t buffer[1];
        };
    }

    struct card
    {
        struct content
        {
            using get_name_t = builtins::string_t*(__fastcall*)(void* self, uint32_t card_id, bool replace_alnum);
            using get_desc_t = builtins::string_t*(__fastcall*)(void* self, uint32_t card_id, bool replace_alnum);
            using get_level_t = uint64_t(__fastcall*)(void* self, uint32_t card_id);
        };
    };

    struct il2cpp
    {
        struct gc
        {
            using register_thread_t = bool(__fastcall*)(void* baseptr);
        };
    };
}