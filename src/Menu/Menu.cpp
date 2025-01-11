// #include "Menu/Menu.h"

// Menu::Menu(const std::vector<MenuItem*> &items) : items(items), currentItem(0) {}

// void Menu::nextItem()
// {
//     if (!items.empty())
//     {
//         currentItem = (currentItem + 1) % items.size();
//     }
// }

// void Menu::previousItem()
// {
//     if (!items.empty())
//     {
//         currentItem = (currentItem == 0) ? items.size() - 1 : currentItem - 1;
//     }
// }

// bool Menu::hasCurrentItem() const
// {
//     return !items.empty();
// }

// void Menu::incrementCurrent()
// {
//     if (!items.empty())
//     {
//         items[currentItem]->updateValueWithin();
//     }
// }

// void Menu::updateEditingDigit() const
// {
//     if (!items.empty())
//     {
//         items[currentItem]->updateEditingDigit();
//     }
// }

// void Menu::resetEditingDigit() const
// {
//     if (!items.empty())
//     {
//         items[currentItem]->resetEditingDigit();
//     }
// }

// void Menu::display()
// {
//     if (!items.empty())
//     {
//         items[currentItem]->display();
//     }
// }