/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/nationalRailBoard/layouts/layoutReplica.hpp
 * Description: Alternative layout for National Rail (Mirroring a platform dot-matrix).
 */

#ifndef NR_LAYOUT_REPLICA_HPP
#define NR_LAYOUT_REPLICA_HPP

#include "../iNationalRailLayout.hpp"

/**
 * @brief Alternative "Replica" National Rail layout.
 *        Demonstrates the ease of swapping layouts in the LGV pattern.
 */
class layoutNrReplica : public iNationalRailLayout {
public:
    layoutNrReplica(appContext* context);
};

#endif // NR_LAYOUT_REPLICA_HPP
