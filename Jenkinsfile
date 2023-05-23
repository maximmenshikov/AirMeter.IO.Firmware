pipeline {
  agent {
    dockerfile {
      filename 'Dockerfile'
      dir 'tools'
    }
  }

  stages {
    stage('Install dependencies') {
      steps {
        sh 'rm -rf esp-idf || return true'
        sh 'rm -rf idf-tools || return true'
        sh 'git clone https://github.com/espressif/esp-idf.git'
      }
    }
    stage('Copy SDK Config') {
      steps {
        sh 'cp boards/esp32-idf/* ./'
      }
    }
    stage('Install ESP-IDF') {
      steps {
        sh '''#!/bin/bash
          export IDF_TOOLS_PATH=$(pwd)/idf-tools
          cd esp-idf
          ./install.sh
        '''
      }
    }
    stage('Disable ESP-IDF Wifi Provisioning') {
      steps {
        sh 'rm esp-idf/components/wifi_provisioning/CMakeLists.txt'
      }
    }
    stage('Build firmware') {
      steps {
        sh '''#!/bin/bash
          export IDF_TOOLS_PATH=$(pwd)/idf-tools
          cd esp-idf
          . ./export.sh
          cd ..
          idf.py build
        '''
      }
    }
  }
}
