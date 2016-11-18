# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

"""Support for bootstrap tasks."""

from __future__ import absolute_import, unicode_literals

from voluptuous import (
    Required,
    Schema,
)

from taskgraph.transforms.job import (
    run_job_using,
)


bootstrap_schema = Schema({
    Required('using'): 'bootstrap',

    # Command to configure system install. This is basically to get system
    # packages updated and to get pre-bootstrap requirements on the system.
    # As part of this, wget should be installed.
    Required('system-setup'): basestring,
    Required('python'): basestring,
})


@run_job_using('docker-worker', 'bootstrap', schema=bootstrap_schema)
def docker_worker_bootstrap(config, job, taskdesc):
    run = job['run']
    worker = taskdesc['worker']
    worker['env'].update({
        'SHELL': '/bin/bash',
    })

    bsurl = '%s/raw-file/%s/python/mozboot/bin/bootstrap.py' % (
        config.params['head_repository'].rstrip('/'),
        config.params['head_rev'])

    command = [
        'bash', '-cx', ' && '.join([
            run['system-setup'],
            'wget %s' % bsurl,
            '%s bootstrap.py --no-interactive --application-choice=browser' % run['python'],
            # Many distros run an ancient Mercurial that doesn't do cloen bundles.
            # So install a modern Mercurial manually to ensure the clone below
            # runs optimally.
            'wget https://bootstrap.pypa.io/get-pip.py',
            '%s get-pip.py' % run['python'],
            'pip install --upgrade Mercurial==4.0',
            'hg clone -U %s repo' % config.params['base_repository'],
            'cd repo',
            'hg pull -r %s %s' % (config.params['head_rev'], config.params['head_repository']),
            'hg up -r %s' % config.params['head_rev'],
            './mach build',
        ])
    ]

    worker['command'] = command
