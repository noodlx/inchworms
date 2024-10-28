pipeline {
    agent { 
        node {
            label 'docker-agent-alpine'
            }
      }
    stages {
        stage('Build') {
            steps {
                make clean
                make all
                echo "Building.."
                sh '''
                echo "doing build stuff.."
                '''
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
                sh '''
                echo "doing SonarQube stuff.."
                '''
            }     
        }
        stage('Deliver') {
            steps {
                make distribute
                echo 'Deliver....'
                sh '''
                echo "doing delivery stuff.."
                '''
            }
        }
    }
}
