/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QBrush>
#include "InputEditorModel.h"


InputEditorModel::InputEditorModel(Context* c, MovieFile* m, QObject *parent) : QAbstractTableModel(parent), context(c), movie(m) {}

int InputEditorModel::rowCount(const QModelIndex & /*parent*/) const
{
   return movie->nbFrames();
}

int InputEditorModel::columnCount(const QModelIndex & /*parent*/) const
{
    return input_set.size();
}

QVariant InputEditorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            return QString(input_set[section].description.c_str());
        }
        if (orientation == Qt::Vertical) {
            return section;
        }
    }
    return QVariant();
}

QVariant InputEditorModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }

    if (role == Qt::BackgroundRole) {
        if (index.row() < context->framecount)
            return QBrush(Qt::gray);
        if (index.row() == context->framecount)
            return QBrush(Qt::lightGray);

        return QBrush(Qt::white);
    }

    if (role == Qt::DisplayRole) {

        const AllInputs ai = movie->input_list[index.row()];
        const SingleInput si = input_set[index.column()];

        /* Check if the single input is set in movie inputs */
        bool is_set = false;

        if (si.type == IT_KEYBOARD) {
            for (const KeySym& ks : ai.keyboard) {
                if (si.value == ks) {
                    is_set = true;
                    break;
                }
            }
        }

        if (si.type & IT_CONTROLLER_ID_MASK) {
            int controller_i = ((si.type & IT_CONTROLLER_ID_MASK) >> IT_CONTROLLER_ID_SHIFT) - 1;
            int controller_axis = si.type & IT_CONTROLLER_AXIS_MASK;
            int controller_type = si.type & IT_CONTROLLER_TYPE_MASK;

            /* We don't support analog inputs in input editor */
            if (!controller_axis) {
                is_set = ai.controller_buttons[controller_i] & ((si.value & 0x1) << controller_type);
            }
        }

        if (is_set) {
            return QString(si.description.c_str());
        }
        else {
            return QString("");
        }
    }
    return QVariant();
}

void InputEditorModel::buildInputSet()
{
    std::set<SingleInput> new_input_set;

    /* Gather all unique inputs from the movie */
    for (const AllInputs &ai : movie->input_list) {
        for (const KeySym& ks : ai.keyboard) {
            if (ks != XK_VoidSymbol) {
                SingleInput si = {IT_KEYBOARD, static_cast<unsigned int>(ks), std::to_string(ks)};
                new_input_set.insert(si);
            }
            else {
                break;
            }
        }
        for (int c = 0; c < AllInputs::MAXJOYS; c++) {
            if (!ai.controller_buttons[c]) {
                continue;
            }
            else {
                for (int b=0; b<16; b++) {
                    if (ai.controller_buttons[c] & (1 << b)) {
                        SingleInput si = {((c+1) << IT_CONTROLLER_ID_SHIFT) + b, 1, ""};
                        new_input_set.insert(si);
                    }
                }
            }
        }
    }

    input_set.clear();
    for (SingleInput si : new_input_set) {

        /* Gather input description */
        for (SingleInput ti : context->config.km.input_list) {
            if (si == ti) {
                si.description = ti.description;
                break;
            }
        }

        /* Insert input */
        input_set.push_back(si);
    }

}

void InputEditorModel::toggleInput(const QModelIndex &index)
{
    /* Don't toggle past inputs */
    if (index.row() < context->framecount)
        return;

    SingleInput si = input_set[index.column()];
    AllInputs &ai = movie->input_list[index.row()];

    if (si.type == IT_KEYBOARD) {
        /* Check if key is set and remove it */
        bool is_set = false;
        int index_set = 0;
        int k;
        for (k=0; k < AllInputs::MAXKEYS; k++) {
            if (ai.keyboard[k] == XK_VoidSymbol) {
                if (is_set) {
                    /* Switch the last set key and the removed key */
                    ai.keyboard[index_set] = ai.keyboard[k-1];
                    ai.keyboard[k-1] = XK_VoidSymbol;
                }
                break;
            }
            if (si.value == ai.keyboard[k]) {
                is_set = true;
                index_set = k;
                ai.keyboard[k] = XK_VoidSymbol;
            }
        }

        /* If not set, add it */
        if (!is_set) {
            if (k < AllInputs::MAXKEYS) {
                ai.keyboard[k] = si.value;
            }
        }
    }

    if (si.type & IT_CONTROLLER_ID_MASK) {
        int controller_i = ((si.type & IT_CONTROLLER_ID_MASK) >> IT_CONTROLLER_ID_SHIFT) - 1;
        int controller_axis = si.type & IT_CONTROLLER_AXIS_MASK;
        int controller_type = si.type & IT_CONTROLLER_TYPE_MASK;

        /* We don't support analog inputs in input editor */
        if (!controller_axis) {
            ai.controller_buttons[controller_i] ^= ((si.value & 0x1) << controller_type);
        }
    }

    emit dataChanged(index, index);
}

std::string InputEditorModel::inputLabel(int column)
{
    return input_set[column].description;
}

void InputEditorModel::renameLabel(int column, std::string label)
{
    input_set[column].description = label;
    emit dataChanged(createIndex(0, column), createIndex(rowCount(), column));
}


std::string InputEditorModel::inputDescription(int column)
{
    SingleInput si = input_set[column];

    /* Gather input description */
    for (SingleInput ti : context->config.km.input_list) {
        if (si == ti) {
            return ti.description;
        }
    }

    return "";
}

bool InputEditorModel::insertRows(int row, int count, const QModelIndex &parent)
{
    /* Don't insert past inputs */
    if (row < context->framecount)
        return false;

    beginInsertRows(parent, row, row+count-1);

    AllInputs ai;
    ai.emptyInputs();

    for (int i=0; i<count; i++) {
        movie->insertInputsBefore(ai, row);
    }

    endInsertRows();

    /* Update the movie framecount. Should it be done here ?? */
    context->config.sc.movie_framecount = movie->nbFrames();
    context->config.sc_modified = true;
    emit frameCountChanged();

    return true;
}

bool InputEditorModel::removeRows(int row, int count, const QModelIndex &parent)
{
    /* Don't delete past inputs */
    if (row < context->framecount)
        return false;

    beginRemoveRows(parent, row, row+count-1);

    for (int i=0; i<count; i++) {
        movie->deleteInputs(row);
    }

    endRemoveRows();

    /* Update the movie framecount. Should it be done here ?? */
    context->config.sc.movie_framecount = movie->nbFrames();
    context->config.sc_modified = true;
    emit frameCountChanged();

    return true;
}


//
//
// void RamSearchModel::searchWatches(CompareType ct, CompareOperator co, double cv)
// {
//     compare_type = ct;
//     compare_operator = co;
//     compare_value = cv;
//
//     beginResetModel();
//
//     int count = 0;
//     ramwatches.erase(
//         std::remove_if(ramwatches.begin(), ramwatches.end(),
//             [this, &count] (std::unique_ptr<IRamWatch> &watch) {
//                 if (!(count++ & 0xfff)) {
//                     emit signalProgress(count);
//                 }
//                 return watch->check_update(compare_type, compare_operator, compare_value);
//             }),
//         ramwatches.end());
//
//     endResetModel();
// }
//

void InputEditorModel::update()
{
    if (input_set.empty()) {
        beginResetModel();
        buildInputSet();
        endResetModel();
    }
    else {
        emit dataChanged(createIndex(0,0), createIndex(rowCount(),columnCount()));
    }
}