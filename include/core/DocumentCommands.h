#pragma once

#include "core/LayerStack.h"
#include "core/HistoryManager.h"

namespace core::commands {

// Centralized layer operations with consistent history recording.
// Eliminates triple-duplication across main.cpp shortcuts, menu bar, and LayersPanel buttons.

void NewLayer(LayerStack& stack, HistoryManager& history, int imgWidth, int imgHeight);
void DuplicateLayer(LayerStack& stack, HistoryManager& history, int selectedIdx);
void DeleteLayer(LayerStack& stack, HistoryManager& history, int selectedIdx);
void MergeDown(LayerStack& stack, HistoryManager& history, int selectedIdx);
void MergeVisible(LayerStack& stack, HistoryManager& history, int width, int height);
void MoveLayerUp(LayerStack& stack, int selectedIdx);
void MoveLayerDown(LayerStack& stack, int selectedIdx);
void SelectLayerUp(LayerStack& stack, int selectedIdx);
void SelectLayerDown(LayerStack& stack, int selectedIdx);

} // namespace core::commands
