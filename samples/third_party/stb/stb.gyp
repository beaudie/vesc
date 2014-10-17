{
    'targets':
    [
        {
            'target_name': 'stb',
            'type': 'static_library',
            'sources':
            [
                'stb_truetype.c',
                'stb_truetype.h',
            ],
            'defines':
            [
               'STB_TRUETYPE_IMPLEMENTATION',
            ],
            'all_dependent_settings':
            {
                'include_dirs':
                [
                    '.',
                ],
            },
        },
    ],
}