from esp_docs.conf_docs import *  # noqa: F403,F401

extensions += [
    'sphinx_copybutton',
    'esp_docs.esp_extensions.dummy_build_system',
    'esp_docs.esp_extensions.run_doxygen',
]

# HTML theme settings
html_theme = 'sphinx_rtd_theme'

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

# Override problematic settings from esp_docs.conf_docs
html_static_path = []  # Remove the _static path since it doesn't exist