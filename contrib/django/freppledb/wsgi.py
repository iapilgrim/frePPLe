# file : $URL$
# revision : $LastChangedRevision$  $LastChangedBy$
# date : $LastChangedDate$
"""
Configuration for frePPLe django WSGI web application.
This is used by the different WSGI deployment options:
  - mod_wsgi on apache web server
  - django development server 'manage.py runserver'
  - fast cgi django server 'manage.py runfcgi'
"""

import os, sys
from django.core.wsgi import get_wsgi_application

# Assure frePPLe is found in the Python path.
#sys.path.append('/home/frepple/workspace/frepple/contrib/django')
#os.environ['FREPPLE_HOME'] = '/home/frepple/workspace/frepple/bin'

os.environ['LC_ALL'] = 'en_US.UTF-8'
os.environ['DJANGO_SETTINGS_MODULE'] = 'freppledb.settings'

application = get_wsgi_application()