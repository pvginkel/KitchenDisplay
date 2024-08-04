library('JenkinsPipelineUtils') _

podTemplate(inheritFrom: 'jenkins-agent-large', containers: [
    containerTemplate(name: 'dockcross', image: 'dockcross/linux-arm64', command: 'sleep', args: 'infinity', alwaysPullImage: true, envVars: [
        containerEnvVar(key: 'BUILDER_UID', value: '1000'),
        containerEnvVar(key: 'BUILDER_GID', value: '1000'),
        containerEnvVar(key: 'BUILDER_USER', value: 'jenkins'),
        containerEnvVar(key: 'BUILDER_GROUP', value: 'jenkins'),
    ]),
    containerTemplate(name: 'dockbuild', image: 'dockbuild/ubuntu2104', command: 'sleep', args: 'infinity', alwaysPullImage: true, envVars: [
        containerEnvVar(key: 'BUILDER_UID', value: '1000'),
        containerEnvVar(key: 'BUILDER_GID', value: '1000'),
        containerEnvVar(key: 'BUILDER_USER', value: 'jenkins'),
        containerEnvVar(key: 'BUILDER_GROUP', value: 'jenkins'),
    ]),
    containerTemplates.rsync('rsync')
]) {
    node(POD_LABEL) {
        stage('Build kitchendisplay') {
            dir('KitchenDisplay') {
                git branch: 'main',
                    credentialsId: '5f6fbd66-b41c-405f-b107-85ba6fd97f10',
                    url: 'https://github.com/pvginkel/KitchenDisplay.git'
                    
                sh 'git rm tools/windows_simulator'
                // We're not pulling down submodules recursively. OpenSSL has a bunch
                // we don't need.
                sh 'git submodule update --init --depth 1'
                
                sh '(cd lib/icu && git rev-parse HEAD) > icu_hash'
                sh '(cd lib/libbacktrace && git rev-parse HEAD) > libbacktrace_hash'
                sh '(cd lib/curl && echo "$(git rev-parse HEAD)-2") > curl_hash'
                sh '(cd lib/openssl && echo "$(git rev-parse HEAD)-2") > openssl_hash'
                sh '(cd lib/zlib && echo "$(git rev-parse HEAD)-2") > zlib_hash'
                
                cache(defaultBranch: 'main', caches: [
                    arbitraryFileCache(path: 'build/lib/icu', cacheValidityDecidingFile: 'icu_hash'),
                    arbitraryFileCache(path: 'build/lib/libbacktrace', cacheValidityDecidingFile: 'libbacktrace_hash'),
                    arbitraryFileCache(path: 'build/lib/curl', cacheValidityDecidingFile: 'curl_hash'),
                    arbitraryFileCache(path: 'build/lib/openssl', cacheValidityDecidingFile: 'openssl_hash'),
                    arbitraryFileCache(path: 'build/lib/zlib', cacheValidityDecidingFile: 'zlib_hash'),
                ]) {
                    container('dockbuild') {
                        sh 'scripts/dockcross/crossbuild.sh prerequisites'
                    }
                    container('dockcross') {
                        sh 'scripts/dockcross/crossbuild.sh'
                    }
                }
            }
            
            dir('HelmCharts') {
                git branch: 'main',
                    credentialsId: '5f6fbd66-b41c-405f-b107-85ba6fd97f10',
                    url: 'https://github.com/pvginkel/HelmCharts.git'
            }
        }
        
        stage('Deploy kitchendisplay') {
            dir('KitchenDisplay') {
                helmCharts.ssh('pvginkel@192.168.178.11', 'sudo systemctl stop kitchendisplay')
                container('rsync') {
                    helmCharts.rsync('bin/.', 'pvginkel@192.168.178.11:/var/local/kitchendisplay/bin')
                }
                helmCharts.ssh('pvginkel@192.168.178.11', 'sudo systemctl start kitchendisplay')
            }
        }
    }
}
