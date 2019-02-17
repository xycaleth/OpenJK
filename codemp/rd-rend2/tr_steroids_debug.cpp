#include "tr_steroids_debug.h"

#include <cstdarg>

#include "tr_allocator.h"
#include "tr_steroids_frame.h"
#include "../rd-common/tr_font.h"

namespace r2
{
    namespace
    {
        struct text_cursor_t
        {
            int fontHandle;
            int x;
            int y;
        };

        struct table_t
        {
            static const int MAX_COLUMN_COUNT = 2;

            int columnCount;
            int columnStarts[MAX_COLUMN_COUNT];

            int fontHandle;

            int y;
            int cellIndex;
        };

        void PlaceCursor(
            text_cursor_t *cursor,
            int x,
            int y,
            int fontHandle)
        {
            *cursor = {};
            cursor->fontHandle = fontHandle;
            cursor->x = x;
            cursor->y = y;
        }

        void DrawText(
            text_cursor_t *cursor,
            float scale,
            const char *text,
            va_list args)
        {
            static const float WHITE[] = {1.0f, 1.0f, 1.0f, 1.0f};
            static const float BLACK[] = {0.0f, 0.0f, 0.0f, 1.0f};

            char fmtText[1024];
            Q_vsnprintf(fmtText, sizeof(fmtText), text, args);

            RE_Font_DrawString(
                cursor->x + 2,
                cursor->y + 2,
                fmtText,
                BLACK,
                cursor->fontHandle,
                -1,
                scale);
            RE_Font_DrawString(
                cursor->x,
                cursor->y,
                fmtText,
                WHITE,
                cursor->fontHandle,
                -1,
                scale);

            cursor->y += 15;
        }

        void DrawText(text_cursor_t *cursor, float scale, const char *text, ...)
        {
            va_list args;
            va_start(args, text);
            DrawText(cursor, scale, text, args);
            va_end(args);
        }

        void TableBeginRow(table_t *table)
        {
            table->cellIndex = 0;
        }

        void TableAddRowCell(table_t *table, float scale, const char *text, ...)
        {
            if (table->cellIndex == table->columnCount)
                return;

            text_cursor_t cursor;
            PlaceCursor(
                &cursor,
                table->columnStarts[table->cellIndex],
                table->y,
                table->fontHandle);

            va_list args;
            va_start(args, text);
            DrawText(&cursor, scale, text, args);
            va_end(args);

            ++table->cellIndex;
        }

        void TableEndRow(table_t *table)
        {
            table->y += 15;
        }
    }

    void DebugInit(debug_t *debug, const frame_t *frame)
    {
        *debug = {};
        debug->fontHandle = RE_RegisterFont("arialnb");
        debug->frame = frame;
    }

    void DebugRender(const debug_t *debug)
    {
        const frame_t *frame = debug->frame;

        const char *base = static_cast<const char *>(frame->memory->Base());
        const char *watermark = static_cast<const char *>(
            frame->memory->Mark());

        const float memoryUsed = (watermark - base) / 1024.0f;
        const float memoryAvailable =
            frame->memory->GetSize() / 1024.0f;

        table_t table = {};
        table.columnCount = 2;
        table.columnStarts[0] = 50;
        table.columnStarts[1] = 150;
        table.fontHandle = debug->fontHandle;
        table.y = 50;

        TableBeginRow(&table);
        {
            TableAddRowCell(&table, 1.0f, "Frame memory:");
            TableAddRowCell(
                &table,
                1.0f,
                "%.2f/%.2f Kb",
                memoryUsed,
                memoryAvailable);
        }
        TableEndRow(&table);

        TableBeginRow(&table);
        {
            TableAddRowCell(&table, 1.0f, "Entity Count:");
            TableAddRowCell(&table, 1.0f, "%d", debug->entityCount);
        }
        TableEndRow(&table);

        TableBeginRow(&table);
        {
            TableAddRowCell(&table, 1.0f, "Culled Surfaces:");
            TableAddRowCell(
                &table,
                1.0f,
                "%d",
                debug->worldSurfaceCount + debug->entitySurfaceCount);
        }
        TableEndRow(&table);

        TableBeginRow(&table);
        {
            TableAddRowCell(&table, 1.0f, "    Entity:");
            TableAddRowCell(
                &table,
                0.8f,
                "%d",
                debug->entitySurfaceCount);
        }
        TableEndRow(&table);

        TableBeginRow(&table);
        {
            TableAddRowCell(&table, 1.0f, "    World:");
            TableAddRowCell(
                &table,
                0.8f,
                "%d",
                debug->worldSurfaceCount);
        }
        TableEndRow(&table);

        TableBeginRow(&table);
        {
            TableAddRowCell(&table, 1.0f, "Scene Render:");
            TableAddRowCell(&table, 1.0f, "%dms", debug->sceneSubmitMsec);
        }
        TableEndRow(&table);

        TableBeginRow(&table);
        {
            TableAddRowCell(&table, 1.0f, "Backend:");
            TableAddRowCell(&table, 1.0f, "%dms", debug->backendMsec);
        }
        TableEndRow(&table);

        TableBeginRow(&table);
        {
            TableAddRowCell(&table, 1.0f, "Draw Calls:");
            TableAddRowCell(&table, 1.0f, "%d", debug->drawCount);
        }
        TableEndRow(&table);
    }
}