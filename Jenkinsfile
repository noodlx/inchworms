pipeline {
    agent { 
        node {
            image 'gcc:latest'
            label 'docker-agent-alpine'
            }
      }
    stages {
        stage('Build') {
            steps {
                sh 'apk add --no-cache make'
                sh '''echo "Building.."'''
                sh 'make clean'
                sh 'make all'

            }
        }
        stage('SonarQube Analysis') {
            steps {
                /*script {
                    def scannerHome = tool 'SonarScanner';
                    withSonarQubeEnv('Sonar-Server') {
                        sh "${scannerHome}/bin/sonar-scanner"
                    }
                }*/
                echo "SonarQube.."
                sh '''echo "doing SonarQube stuff.."'''
            }     
        }
        stage('Deliver') {
            steps {
                make distribute
                sh '''echo "doing delivery stuff.."'''
            }
        }
    }
}
