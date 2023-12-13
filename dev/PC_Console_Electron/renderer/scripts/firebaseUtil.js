// firebaseUtil.js

// init the osc client
function firebaseInit() {
    const { initializeApp } = require('firebase/app');
    const { getFirestore, collection, addDoc, getDocs, Firestore, doc, setDoc, Timestamp, updateDoc, serverTimestamp, getDoc, where, query, onSnapshot } = require('firebase/firestore');

    const firebaseConfig = {
        apiKey: "AIzaSyCpPOh4iI--Zlgs96hB4BFq8n9rGL3y4ks",
        authDomain: "rtc-server-5d1cf.firebaseapp.com",
        projectId: "rtc-server-5d1cf",
        storageBucket: "rtc-server-5d1cf.appspot.com",
        messagingSenderId: "689362156688",
        appId: "1:689362156688:web:a2bb68d1624758c56e2c15",
        measurementId: "G-4DNL0Y0BFR"
    };
    const app = initializeApp(firebaseConfig);
    const servers = {
        iceServers: [
            {
                urls: ['stun:stun1.l.google.com:19302', 'stun:stun2.l.google.com:19302'],
            },
        ],
        iceCandidatePoolSize: 10,
    };
    const db = getFirestore(app);
    return { firebaseConfig, app, servers, db };
}

// Export the functions so they can be used in other modules or the main application.
export { firebaseInit };
