library('JenkinsPipelineUtils') _

podTemplate(inheritFrom: 'jenkins-agent-large', containers: [
    containerTemplate(name: 'dockcross', image: 'dockcross/linux-arm64-full', command: 'sleep', args: 'infinity', alwaysPullImage: true, envVars: [
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
    ])
]) {
    node(POD_LABEL) {
        stage('Build kitchendisplay') {
            dir('KitchenDisplay') {
                git branch: 'main',
                    credentialsId: '5f6fbd66-b41c-405f-b107-85ba6fd97f10',
                    url: 'https://github.com/pvginkel/KitchenDisplay.git'
                    
                sh 'git rm tools/windows_simulator'
                sh 'git submodule update --init --recursive --depth 1'
                
                sh '(cd lib/icu && git rev-parse HEAD) > icu_hash'
                sh '(cd lib/libbacktrace && git rev-parse HEAD) > libbacktrace_hash'
                sh '(cd lib/curl && git rev-parse HEAD) > curl_hash'
                
                cache(defaultBranch: 'main', caches: [
                    arbitraryFileCache(path: 'build/lib/icu', cacheValidityDecidingFile: 'icu_hash'),
                    arbitraryFileCache(path: 'build/lib/libbacktrace', cacheValidityDecidingFile: 'libbacktrace_hash'),
                    arbitraryFileCache(path: 'build/lib/curl', cacheValidityDecidingFile: 'curl_hash')
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
                helmCharts.ssh('pvginkel@iotkitchendisplay.home', 'sudo systemctl stop kitchendisplay')
                helmCharts.scp('bin/.', 'pvginkel@iotkitchendisplay.home:/var/local/kitchendisplay/bin')
                helmCharts.ssh('pvginkel@iotkitchendisplay.home', 'sudo systemctl start kitchendisplay')
            }
        }
    }
}
