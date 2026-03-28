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
public:
    struct WidgetEntry {
        iGfxWidget* widget;
        std::string typeClass;
    };

private:
    std::map<std::string, WidgetEntry> widgets;

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
     * @param typeClass The exact C++ derived class of this widget map
     */
    void registerWidget(const std::string& id, iGfxWidget* widget, const std::string& typeClass = "") {
        widgets[id] = {widget, typeClass};
    }

    /**
     * @brief Retrieves a widget object reference by its string ID
     * @param id The layout JSON component ID string
     * @return iGfxWidget pointer, or nullptr if not mapped
     */
    iGfxWidget* getWidget(const std::string& id) {
        auto it = widgets.find(id);
        if (it != widgets.end()) {
            return it->second.widget;
        }
        return nullptr;
    }

    /**
     * @brief Returns a standard map containing all mapped components and their types
     * @return Const reference to the registry mappings map
     */
    const std::map<std::string, WidgetEntry>& getAllEntries() const {
        return widgets;
    }

    /**
     * @brief Returns a standard map containing all mapped components (for layoutParser compatibility if needed)
     */
    std::map<std::string, iGfxWidget*> getAllWidgets() const {
        std::map<std::string, iGfxWidget*> res;
        for (const auto& pair : widgets) {
            res[pair.first] = pair.second.widget;
        }
        return res;
    }

    /**
     * @brief Wipes all component references from the registry
     */
    void clear() {
        widgets.clear();
    }
};

#endif // DESIGNER_REGISTRY_HPP
