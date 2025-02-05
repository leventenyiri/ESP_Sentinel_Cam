from esp_docs.conf_docs import *  # noqa: F403,F401

extensions += [
    'sphinx_copybutton',
    # Needed as a trigger for running doxygen
    'esp_docs.esp_extensions.dummy_build_system',
    'esp_docs.esp_extensions.run_doxygen',
]

# Base URLs and repository settings
github_repo = 'ESP_Sentinel_Cam'
github_user = 'leventenyiri'
github_url = f'https://github.com/{github_user}/{github_repo}'
BASE_URL = github_url + '/blob/main'

# Project information
project = 'ESP_Sentinel_Cam'
author = 'Levente Nyiri'
copyright = '2024, ' + author

# Repository settings for ESP-Docs
github_repo = 'ESP_Sentinel_Cam'
html_context['github_user'] = 'leventenyiri'
html_context['github_repo'] = github_repo
html_context['github_version'] = 'main'

# Base URL settings
html_baseurl = f'https://leventenyiri.github.io/{github_repo}/'

# Documentation settings
languages = ['en']
language = 'en'
todo_include_todos = True
