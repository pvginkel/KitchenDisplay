library('JenkinsPipelineUtils') _

podTemplate(inheritFrom: 'jenkins-agent-large', containers: [
    containerTemplates.dockbuild('dockcross', 'dockcross/linux-arm64'),
    containerTemplates.dockbuild('dockbuild', 'dockbuild/ubuntu2104'),
    containerTemplates.rsync('rsync')
]) {
    node(POD_LABEL) {
        stage('Build kitchendisplay') {
            dir('KitchenDisplay') {
                git branch: 'main',
                    credentialsId: '5f6fbd66-b41c-405f-b107-85ba6fd97f10',
                    url: 'https://github.com/pvginkel/KitchenDisplay.git'
                
                cache(defaultBranch: 'main', caches: [
                    arbitraryFileCache(path: 'build/lib/icu', cacheValidityDecidingFile: gitUtils.getTreeHashFile('lib/icu')),
                    arbitraryFileCache(path: 'build/lib/libbacktrace', cacheValidityDecidingFile: gitUtils.getTreeHashFile('lib/libbacktrace')),
                    arbitraryFileCache(path: 'build/lib/curl', cacheValidityDecidingFile: gitUtils.getTreeHashFile('lib/curl', '-2')),
                    arbitraryFileCache(path: 'build/lib/openssl', cacheValidityDecidingFile: gitUtils.getTreeHashFile('lib/openssl', '-2')),
                    arbitraryFileCache(path: 'build/lib/zlib', cacheValidityDecidingFile: gitUtils.getTreeHashFile('lib/zlib', '-2')),
                    arbitraryFileCache(path: 'build/lib/lvgl', cacheValidityDecidingFile: gitUtils.getTreeHashFile('lib/lvgl')),
                ]) {
                    container('dockbuild') {
                        sh 'gosu 1000:1000 scripts/dockcross/crossbuild.sh prerequisites'
                    }
                    container('dockcross') {
                        sh 'gosu 1000:1000 scripts/dockcross/crossbuild.sh'
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
