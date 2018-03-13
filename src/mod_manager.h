#pragma once
#ifndef MOD_MANAGER_H
#define MOD_MANAGER_H

#include "dependency_tree.h"
#include "string_id.h"

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>

const std::vector<std::pair<std::string, std::string> > &get_mod_list_categories();

struct WORLD;
typedef WORLD *WORLDPTR;

class mod_ui;
class game;
class worldfactory;
class JsonObject;

const std::vector<std::pair<std::string, std::string> > &get_mod_list_categories();
const std::vector<std::pair<std::string, std::string> > &get_mod_list_tabs();
const std::map<std::string, std::string> &get_mod_list_cat_tab();

struct MOD_INFORMATION;
using mod_id = string_id<MOD_INFORMATION>;

struct MOD_INFORMATION {
    std::string name;
    mod_id ident;

    /** Directory to load JSON and Lua from relative to directory containing modinfo.json */
    std::string path;

    /** If set load legacy migrations from this location dependent upon save version */
    std::string legacy;

    /** All authors who have added content to the mod (excluding maintenance changes) */
    std::set<std::string> authors;

    /**
     *  Assigned maintainers responsible for maintaining compatibility with core
     *  @note these should be verbatim GH account names
     */
    std::set<std::string> maintainers;

    std::string description;
    std::string version;

    /** What other mods must be loaded prior to this one? */
    std::set<mod_id> dependencies;

    /** Core mods are loaded before any other mods */
    bool core = false;

    /** Obsolete mods are loaded for legacy saves but cannot be used when starting new worlds */
    bool obsolete = false;

    /** Does this mod require Lua support? **/
    bool need_lua() const;

    std::pair<int, std::string> category = { -1, "" };
};

class mod_manager
{
    public:
        typedef std::vector<mod_id> t_mod_list;

        mod_manager();
        /**
         * Reload the map of available mods (@ref mod_map).
         * This also reloads the dependency tree.
         */
        void refresh_mod_list();

        /**
         * Returns the dependency tree for the loaded mods.
         * @returns @ref dependency_tree
         */
        dependency_tree &get_tree();
        /**
         * Clear @ref mod_map and delete @ref dependency_tree.
         */
        void clear();
        /**
         * Copy the json files of the listed mods into the
         * given folder (output_base_path)
         * @param mods_to_copy A list (of idents) of mods whose
         * json data should be copied.
         * @param output_base_path The folder where to put the copies.
         * The function creates a sub folder for each mod and
         * puts the files there. The original folder structure is
         * retained.
         */
        bool copy_mod_contents( const t_mod_list &mods_to_copy, const std::string &output_base_path );

        /**
         * Save list of mods that are active in that world to
         * the world folder.
         */
        void save_mods_list( WORLDPTR world ) const;
        /**
         * Load list of mods that should be active in that
         * world.
         */
        void load_mods_list( WORLDPTR world ) const;
        const t_mod_list &get_default_mods() const;
        bool set_default_mods( const t_mod_list &mods );
    protected:
    private:
        // Make this accessible for now
        friend class mod_ui;
        friend class worldfactory;
        friend class game;
        friend mod_id;
        /**
         * @returns path of a file in the world folder that contains
         * the list of mods that should be loaded for this world.
         */
        static std::string get_mods_list_file( const WORLDPTR world );
        /**
         * Load all modinfo.json files (recursively) from the
         * given root.
         * @param path The root folder from which the modinfo
         * files are searched.
         */
        void load_mods_from( std::string path );
        /**
         * Load all mod information from a json file.
         * (@see load_modfile)
         */
        void load_mod_info( std::string info_file_path );
        /**
         * Load mod info from a json object. Put the loaded modinfo
         * directly into @ref mod_map.
         * @throws std::string on all kind of errors. The string
         * contains the error message.
         */
        void load_modfile( JsonObject &jo, const std::string &path );

        bool set_default_mods( const mod_id &ident );
        void remove_mod( const mod_id &ident );
        void remove_invalid_mods( std::vector<mod_id> &mods ) const;

        dependency_tree tree;

        /**
         * The map of known mods, key is the mod ident.
         */
        std::map<mod_id, MOD_INFORMATION> mod_map;
        t_mod_list default_mods;
};

class mod_ui
{
    public:
        mod_ui( mod_manager &modman );

        std::vector<mod_id> usable_mods;
        std::string get_information( const MOD_INFORMATION *mod );
        mod_manager &active_manager;
        dependency_tree &mm_tree;

        void try_add( const mod_id &mod_to_add,
                      std::vector<mod_id> &active_list );
        void try_rem( size_t selection, std::vector<mod_id> &active_list );
        void try_shift( char direction, size_t &selection, std::vector<mod_id> &active_list );

        bool can_shift_up( long selection, std::vector<mod_id> active_list );
        bool can_shift_down( long selection, std::vector<mod_id> active_list );

    private:
        void set_usable_mods();
};

#endif
