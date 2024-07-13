#pragma once

#define CODE_FOLDER L"\\Ransom-Mon-Driver\\"
#define BACKUP_FOLDER L"\\zzbackup\\"
#define DATASET_FOLDER L"\\MarauderMap\\"

#define WHILE_LIST_SIZE 3
extern inline const unsigned short* white_list[WHILE_LIST_SIZE] = { CODE_FOLDER, BACKUP_FOLDER, DATASET_FOLDER };

namespace ransom
{
	extern inline bool is_enabled = true;
	extern inline bool test_mode = false;
}

