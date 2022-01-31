#pragma once

#include "win.hpp"
#include "detail.hpp"
#include "minhook/MinHook.h"
#include "json.hpp"
#include <iostream>
#include <thread>
#include <fstream>
#include <map>

using namespace detail;

struct ygo_card
{
    std::string name;
    std::string desc;
    uint64_t level;
};

struct exodia
{
private:
    static inline uintptr_t base;
    static inline card::content::get_name_t card_get_name;
    static inline card::content::get_desc_t card_get_desc;
    static inline card::content::get_level_t card_get_level;
    static inline il2cpp::gc::register_thread_t register_thread;

    template <typename Pointer, typename Offset>
    static Pointer calc_ptr(Offset offset)
    {
        return reinterpret_cast<Pointer>(
            base + static_cast<uintptr_t>(offset));
    }

    static void __fastcall dump_cards(void* self)
    {
        // The il2cpp vm expects each thread that receives
        // garbage collection to be registered.

        int baseptr = 0;
        register_thread(&baseptr);

        Sleep(5000);

        puts("Starting to dump cards");

        std::map<uint32_t, ygo_card> cards;
        for (uint16_t id = 0; id < (uint16_t)-1; id++)
        {
            auto str = card_get_name_orginal(self, id, 0);
            if (str == nullptr || str->length <= 0)
                continue;

            std::wstring wname(str->buffer);
            printf("%d: %ls\n", id, wname.data());

            str = card_get_desc(self, id, 0);
            std::wstring wdesc(str->buffer);

            auto name = std::string(wname.begin(), wname.end());
            auto desc = std::string(wdesc.begin(), wdesc.end());
            auto level = card_get_level(self, id);
            auto card = ygo_card
            {
                .name = std::move(name),
                .desc = std::move(desc),
                .level = level
            };
            cards[id] = std::move(card);
        }

        std::ofstream out("cards.txt");
        for (const auto& [id, card] : cards)
            out << id << ": " << card.name << std::endl;
        out.close();

        using namespace nlohmann;
        auto arr = json::array();
        for (const auto& [id, card] : cards)
        {
            auto obj = json::object();
            obj["name"] = card.name;
            obj["desc"] = card.desc;
            obj["level"] = card.level;
            arr.push_back(obj);
        }

        out = std::ofstream("cards.json");
        out << arr.dump(4, ' ', false, nlohmann::detail::error_handler_t::ignore);
        out.close();

        puts("Finished dumping cards");
    }

    static inline decltype(card_get_name) card_get_name_orginal = nullptr;
    static inline bool dumped = false;
    static builtins::string_t* __fastcall card_get_name_hk(void* self, uint32_t card_id, bool replace_alnum)
    {
        if (!dumped)
        {
            dumped = true;
            //std::thread(dump_cards, self).detach();
        }

        const auto name = card_get_name_orginal(self, card_id, replace_alnum);
        wprintf(L"Content::Card::GetName(%p, %02x, %d) -> %ls\n", self, card_id, replace_alnum, name->buffer);

        return name;
    }

public:
    static void initialize()
    {
        AllocConsole();
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        system("cls");

        base = win::find_ygo_module();
        printf("GameAssembly.dll @ %p\n", base);

        card_get_name = calc_ptr<decltype(card_get_name)>(offsets::card_content_get_name);
        printf("Card::Content::GetName @ %p\n", card_get_name);

        card_get_desc = calc_ptr<decltype(card_get_desc)>(offsets::card_content_get_desc);
        printf("Card::Content::GetDesc @ %p\n", card_get_desc);

        card_get_level = calc_ptr<decltype(card_get_level)>(offsets::card_content_get_level);
        printf("Card::Content::GetLevel @ %p\n", card_get_level);

        register_thread = calc_ptr<decltype(register_thread)>(offsets::gc_register_thread);
        printf("GarbageCollector::RegisterThread @ %p\n", register_thread);

        MH_Initialize();
        MH_CreateHook(card_get_name, card_get_name_hk, reinterpret_cast<LPVOID*>(&card_get_name_orginal));
        MH_EnableHook(card_get_name);
    }   
};