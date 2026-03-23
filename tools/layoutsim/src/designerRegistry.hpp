/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: tools/layoutsim/src/designerRegistry.hpp
 * Description: Registry for mapping string IDs to iGfxWidget instances.
 *
 * Exported Functions/Classes:
 * - DesignerRegistry: Singleton mapping system for layout IDE component integration
 *   - getInstance(): Returns the global runtime registry instance
 *   - registerWidget(): Maps a string ID to an instantiated graphics widget
 *   - getWidget(): Retrieves a widget object reference by its string ID
 *   - getAllWidgets(): Returns a standard map containing all mapped components
 *   - clear(): Wipes all component references from the registry
 */

#ifndef DESIGNER_REGISTRY_HPP
#define DESIGNER_REGISTRY_HPP

#include <map>
#include <string>
#include "iGfxWidget.hpp"

class DesignerRegistry {
private:
    std::map<std::string, iGfxWidget*> widgets;

public:
    /**
     * @brief Returns the global runtime registry instance
     * @return Reference to the singleton DesignerRegistry
     */
    static DesignerRegistry& getInstance() {
        static DesignerRegistry instance;
        return instance;
    }

    /**
     * @brief Maps a string ID to an instantiated graphics widget
     * @param id The layout JSON component ID string
     * @param widget The generic widget pointer assigned to this ID
     */
    void registerWidget(const std::string& id, iGfxWidget* widget) {
        widgets[id] = widget;
    }

    /**
     * @brief Retrieves a widget object reference by its string ID
     * @param id The layout JSON component ID string
     * @return iGfxWidget pointer, or nullptr if not mapped
     */
    iGfxWidget* getWidget(const std::string& id) {
        auto it = widgets.find(id);
        if (it != widgets.end()) {
            return it->second;
        }
        return nullptr;
    }

    /**
     * @brief Returns a standard map containing all mapped components
     * @return Const reference to the registry mappings map
     */
    const std::map<std::string, iGfxWidget*>& getAllWidgets() const {
        return widgets;
    }

    /**
     * @brief Wipes all component references from the registry
     */
    void clear() {
        widgets.clear();
    }
};

#endif // DESIGNER_REGISTRY_HPP
