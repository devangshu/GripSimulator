/* GRIP SIMULATOR */
// web client


var main, ws, ui, phalanx1finger1;
var myChart, chartHolder, data;
var count = 0;
var finger0phalanx1, finger0phalanx2, finger0phalanx3, finger1phalanx1, finger1phalanx2, finger1phalanx3, finger2phalanx1, finger2phalanx2, finger2phalanx3, finger3phalanx1, finger3phalanx2, finger3phalanx3, finger4phalanx1, finger4phalanx2, finger4phalanx3;

var vp0, vp1, vp2, vp3, vp4, vp5;
var vpvals = new Array();
var x1 = x2 = x3 = .01;



ui = {
    colors: {},
    update_virtual_pin: (vpin_num, vpin_val) => {
        ui.log(`updating ui for vp${vpin_num}: ${vpin_val}`);
        document.querySelector(`#vp_output #vp_pin_${vpin_num} .vp_pin_code_output`).innerHTML = (`${vpin_val}`);
        // TODO: Update graph here
        ui.log(myChart);
        if(count >= 5){
            myChart.data.labels.push("");
            myChart.data.datasets[vpin_num].data.push(vpin_val);
            myChart.update('none');
        }
        //TODO: Update this to be derivative based (for jitter calculation)
        // if(vpin_num == 0){
        //     vp0 = vpin_val;
        // } else if(vpin_num == 1){
        //     vp1 = vpin_val;
        // } else if(vpin_num == 2){
        //     vp2 = vpin_val;
        // } else if(vpin_num == 3){
        //     vp3 = vpin_val;
        // } else if(vpin_num == 4){
        //     vp4 = vpin_val;
        // }  else if(vpin_num == 5){
        //     vp5 = vpin_val;
        // } 

        count++;

    },
    show_login_error_message: _ => {
        document.querySelector("#main_login_form #error_msg").style.opacity = "0.9";
    },
    hide_login_error_message: _ => {
        document.querySelector("#main_login_form #error_msg").style.opacity = "0";
    },
    switch_to_main_view: (load_initial_virtualpin_values = true) => {
        document.querySelector("#main_disconnect_view").style.display = "none";
        document.querySelector("#main_login_form").style.display = "none";
        document.querySelector("#main_app_view").style.display = "block";
        util.delay(_ => {
            if (load_initial_virtualpin_values)
                ws.api.request_vpin_values();
        }, 100);
    },
    switch_to_disconnect_view: _ => {
        document.querySelector("#main_disconnect_view").style.display = "block";
        document.querySelector("#main_login_form").style.display = "none";
        document.querySelector("#main_app_view").style.display = "none";
    },
    init: (resolve) => {
        ui.log('initializing');
        document.querySelector("#main_content_loading").style.display = "none";
        var handle_submit = _ => {
            var value = (`${document.querySelector("#auth_pass").value}`).trim();
            if (value != "") ws.api.login(value);
        };
        $("#auth_pass").on("keyup", e => {
            if (e.keyCode == 13) handle_submit();
        });
        $("#auth_submit").click(e => {
            handle_submit();
        });
        $("#navbar #logout button").click(e => {
            ws.api.logout();
        });
        $("#main_disconnect_view #disconnect_reload").click(e => {
            window.location.reload();
        });
        util.delay(_ => {
            document.querySelector("#main_content_main").style.display = "block";
        }, 10);
        document.querySelector("#main_app_view #vp_output").innerHTML = "";
        const labels = [
            'Time',
        ];

        data = {
            labels: labels,
            datasets: [{
                label: 'VP0',
                backgroundColor: 'rgb(255, 199, 132)',

                data: [config.virtualpin_value_init],
            },
            {
                label: 'VP1',
                backgroundColor: 'rgb(155, 29, 132)',

                data: [config.virtualpin_value_init],
            },
            {
                label: 'VP2',
                backgroundColor: 'rgb(25, 89, 132)',

                data: [config.virtualpin_value_init],
            },
            {
                label: 'VP3',
                backgroundColor: 'rgb(235, 159, 132)',

                data: [config.virtualpin_value_init],
            },
            {
                label: 'VP4',
                backgroundColor: 'rgb(215, 189, 132)',
                data: [config.virtualpin_value_init],
            }
            ]
        };

        const chartConfig = {
            type: 'line',
            data: data,
            options: {
                responsive: false,
            }
        };
        myChart = new Chart(
            document.getElementById('myChart'),
            chartConfig
        );
        var vp_output_html = "";
        for (var v = 0; v < config.virtualpin_count; v++) {
            vp_output_html += `<div id="vp_pin_${v}">VP${v}&nbsp;&nbsp;=&nbsp;&nbsp;<code class="vp_pin_code_output">${config.virtualpin_value_init}</code></div>`;
        }
        //TODO: maybe update stuff here

        document.querySelector("#main_app_view #vp_output").innerHTML = vp_output_html;
        if (resolve) resolve();
    },
    log: util.logger('ui'),
    err: util.logger('ui', true)
};

ws = {
    id: 0,
    socket: null,
    events: [],
    url: (location.protocol === 'https:' ? 'wss://' : 'ws://') + document.domain +
        (document.domain == 'localhost' ? ':8080' : ((location.protocol === 'https:' ? ':443' : ':80') + '/socket')),
    bind_events: _ => {
        ws.log("binding events");
        ws.bind('auth', success => {
            if (success) {
                ui.hide_login_error_message();
                ui.switch_to_main_view();
            } else {
                ui.show_login_error_message();
            }
        });
        ws.bind('vpin_values', value_map => {
            var i = 0;
            for (var v in value_map) {
                ui.update_virtual_pin(value_map[v]._id, value_map[v].value);
                vpvals[i] = value_map[v].value;
                i++; 
            }
        });
        for (var v = 0; v < config.virtualpin_count; v++) {
            ws.bind(`${config.virtualpin_topic}_${v}`, ws.generate_virtualpin_handler(v));
        }
    },
    generate_virtualpin_handler: vp_num => {
        return (vp_value) => {
            main.receive_virtual_pin_update(vp_num, parseInt(vp_value), vp_value);
        };
    },
    encode_msg: (e, d) => {
        return JSON.stringify({
            event: e,
            data: d
        });
    },
    decode_msg: (m) => {
        try {
            m = JSON.parse(m);
        } catch (e) {
            ws.err('invalid json ', e);
            m = null;
        }
        return m;
    },
    bind: (event, handler) => {
        ws.events[event] = handler;
    },
    connect: resolve => {
        ws.log('connecting');
        var socket = new WebSocket(ws.url);
        socket.addEventListener('open', _ => {
            ws.log(`socket connected to ${ws.url}`);
            if (resolve) resolve();
        });
        socket.addEventListener('error', e => {
            ws.err('socket error', e.data);
        });
        socket.addEventListener('message', e => {
            var d = ws.decode_msg(e.data);
            if (d != null) {
                ws.log('socket received:', d.event, d.data);
                if (ws.events.hasOwnProperty(d.event) && ws.events[d.event])
                    ws.events[d.event](d.data);
            } else {
                ws.err('socket received:', 'invalid message', e.data);
            }
        });
        socket.addEventListener('close', e => {
            ws.log('socket disconnected');
            ws.api.disconnect();
            // alert('disconnected from server');
        });
        window.addEventListener('beforeunload', e => {
            // socket.close(1001);
        });
        hand.init();
        ws.socket = socket;
        ws.bind_events();
    },
    send: (event, data) => {
        ws.log('socket sending:', event, data);
        ws.socket.send(ws.encode_msg(event, data));
    },
    api: {
        cookie_login_flag: false,
        cookie_login: _ => {
            var hash_cookie = util.cookie('password');
            if (hash_cookie != null && (`${hash_cookie}`).trim() != "") {
                ws.api.cookie_login_flag = true;
                hash_cookie = (`${hash_cookie}`).trim();
                ws.send('auth', {
                    password: hash_cookie
                });
            }
        },
        login: password => {
            util.sha256(`${password}`, hash => {
                ws.send('auth', {
                    password: `${hash}`
                });
                util.cookie('password', `${hash}`, '__indefinite__');
            });
        },
        logout: _ => {
            util.delete_cookie('password');
            window.location.reload();
        },
        disconnect: _ => {
            ui.switch_to_disconnect_view();
        },
        request_vpin_values: _ => {
            ws.send('get_vpin_values');
        }
    },
    log: util.logger('ws'),
    err: util.logger('ws', true)
};

main = {
    test: _ => {
        // TODO: add any necessary web client tests here
        // main.log('testing', '123');
    },
    virtual_pins: [], // local pin values
    receive_virtual_pin_update: (vp_num, vp_val, vp_val_buffer = "") => {
        if (vp_num < 0 || vp_num >= config.virtualpin_count) {
            main.err(`invalid virtual pin update on vp?: invalid pin number ${vp_num}`);
            return;
        }
        if (vp_num == NaN) {
            main.err(`invalid virtual pin update on vp${vp_num}: unable to parse value ${vp_val_buffer}`);
            return;
        }
        // check if update is new
        if (vp_val != main.virtual_pins[vp_num]) {
            main.log(`virtual pin update on vp${vp_num}: ${vp_val}`);
            // update local value
            main.virtual_pins[vp_num] = vp_val;
            // update ui elements
            ui.update_virtual_pin(vp_num, vp_val);
            vpvals[vp_num] = vp_val;       
        }
    },
    main: _ => {
        util.delay(_ => {
            main.init(_ => {
                util.delay(main.test, 500);
            });
        }, 100);
    },
    init: resolve => {
        console.clear();
        main.log('loading');
        for (var v = 0; v < config.virtualpin_count; v++)
            main.virtual_pins[v] = config.virtualpin_value_init;
        util.delay(_ => {
            main.log('socket connecting');
            ws.connect(_ => {
                util.delay(_ => {
                    ui.init(_ => {
                        main.log('ready');
                        util.delay(ws.api.cookie_login, 200);
                        if (resolve) resolve();

                    });
                }, 100);
            });
        }, 50);

    },
    log: util.logger('main'),
    err: util.logger('main', true)
};

hand = {
    scene: THREE.Scene,
    camera: THREE.Camera,
    renderer: THREE.WebGLRenderer,
    group: THREE.Object3D,
    test: _ => {
        hand.log("test test");
    },
    init: (resolve) => {
        hand.test();
        var threeCanvas = document.body.getElementsByClassName("model")[0];

        if (threeCanvas) {
            hand.log("test 3000");
        }
        // main.log("Test");

        scene = new THREE.Scene();
        camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 1, 1000);

        const renderer = new THREE.WebGLRenderer({ alpha: true });
        renderer.setSize(window.innerWidth / 2, window.innerHeight / 2);
        // renderer.setClearColor(THREE.Color.NAMES.clear, 0);
        document.body.getElementsByClassName("model")[0].appendChild(renderer.domElement);

        clock = new THREE.Clock();

        const geometry = new THREE.BoxGeometry();
        const material = new THREE.MeshBasicMaterial({ color: 0x00ff00 });
        const cube = new THREE.Mesh(geometry, material);
        // scene.add(cube);

        // camera = new THREE.PerspectiveCamera( 75, window.innerWidth / window.innerHeight, 1, 1000 );
        camera.position.z = 30;
        hand.drawHand();

        const animate = function () {
            requestAnimationFrame(animate);
            //TODO: Update each phalanx here 

            var val0 = ( (vpvals[0] - 30) /100) * 1.5;
            finger0phalanx1.rotation.x = val0;
            finger0phalanx2.rotation.x = val0;

            var val1 = (vpvals[1]/100) * 1.5;
            finger1phalanx1.rotation.x = val1;
            finger1phalanx2.rotation.x = val1;
            finger1phalanx3.rotation.x = val1;

            var val2 = ( Math.abs(vpvals[0]/ (100))) * 1.4;
            
            finger2phalanx1.rotation.x = val2;
            finger2phalanx2.rotation.x = val2;
            finger2phalanx3.rotation.x = val2;

            var val3 = (vpvals[3]/100) * 1.5;
            finger3phalanx1.rotation.x = val3;
            finger3phalanx2.rotation.x = val3;
            finger3phalanx3.rotation.x = val3;

            var val4 = (vpvals[4]/100) * 1.5;
            finger4phalanx1.rotation.x = val4;
            finger4phalanx2.rotation.x = val4;
            finger4phalanx3.rotation.x = val4;

            // group.rotation.y += 0.01;
            group.rotation.y += 0.01;


            renderer.render(scene, camera);
        };

        animate();
    },
    drawHand: _ => {
        group = new THREE.Object3D();
        group.position.set(0, 0, 0);

        // var falangeP1 = new THREE.Vector3(10, 30, 10);
        // var falangeP2 = new THREE.Vector3(-10, 0, -10);

        const palmGeometry = new THREE.BoxGeometry(10, 10, 1);
        const fingerGeometry = new THREE.BoxGeometry(1, 5, 1);
        const thumbGeometry = new THREE.BoxGeometry(1, 3, 1);
        const palmMaterial = new THREE.MeshBasicMaterial({ color: 0xFF0000 });
        const fingerMaterial = new THREE.MeshBasicMaterial({ color: 0x00ff00 });
        const palm = new THREE.Mesh(palmGeometry, palmMaterial);

        //thumb
        finger0phalanx1 = new THREE.Mesh(thumbGeometry, fingerMaterial);
        finger0phalanx2 = new THREE.Mesh(thumbGeometry, fingerMaterial);
        // finger1phalanx3 = new THREE.Mesh(fingerGeometry, fingerMaterial);

        //Index
        finger1phalanx1 = new THREE.Mesh(fingerGeometry, fingerMaterial);
        finger1phalanx2 = new THREE.Mesh(fingerGeometry, fingerMaterial);
        finger1phalanx3 = new THREE.Mesh(fingerGeometry, fingerMaterial);
        
        //Middle
        finger2phalanx1 = new THREE.Mesh(fingerGeometry, fingerMaterial);
        finger2phalanx2 = new THREE.Mesh(fingerGeometry, fingerMaterial);
        finger2phalanx3 = new THREE.Mesh(fingerGeometry, fingerMaterial);
        
        //Ring? Wow i dnot know the names of the fingers
        finger3phalanx1 = new THREE.Mesh(fingerGeometry, fingerMaterial);
        finger3phalanx2 = new THREE.Mesh(fingerGeometry, fingerMaterial);
        finger3phalanx3 = new THREE.Mesh(fingerGeometry, fingerMaterial);

        //pinky
        finger4phalanx1 = new THREE.Mesh(fingerGeometry, fingerMaterial);
        finger4phalanx2 = new THREE.Mesh(fingerGeometry, fingerMaterial);
        finger4phalanx3 = new THREE.Mesh(fingerGeometry, fingerMaterial);

        finger0phalanx1.position.x = -6;
        finger0phalanx1.position.y = 2;
        finger0phalanx1.position.z = 0;
        finger0phalanx1.rotation.z = 1;
        finger0phalanx2.position.y = 3;


        finger1phalanx1.position.x = -5;
        finger1phalanx1.position.y = 4;
        finger1phalanx1.position.z = 0;
        finger1phalanx2.position.y = 4;
        finger1phalanx3.position.y = 4;

        finger2phalanx1.position.x = -2;
        finger2phalanx1.position.y = 4;
        finger2phalanx1.position.z = 0;
        finger2phalanx2.position.y = 4;
        finger2phalanx3.position.y = 4;

        finger3phalanx1.position.x = 1;
        finger3phalanx1.position.y = 4;
        finger3phalanx1.position.z = 0;
        finger3phalanx2.position.y = 4;
        finger3phalanx3.position.y = 4;

        finger4phalanx1.position.x = 4;
        finger4phalanx1.position.y = 4;
        finger4phalanx1.position.z = 0;
        finger4phalanx2.position.y = 4;
        finger4phalanx3.position.y = 4;



        fingerGeometry.computeVertexNormals();
        
        group.add(palm);

        finger0phalanx1.add(finger0phalanx2);
        group.add(finger0phalanx1);
        
        finger1phalanx2.add(finger1phalanx3);
        finger1phalanx1.add(finger1phalanx2);
        group.add(finger1phalanx1);

        finger2phalanx2.add(finger2phalanx3);
        finger2phalanx1.add(finger2phalanx2);
        group.add(finger2phalanx1);

        finger3phalanx2.add(finger3phalanx3);
        finger3phalanx1.add(finger3phalanx2);
        group.add(finger3phalanx1);

        finger4phalanx2.add(finger4phalanx3);
        finger4phalanx1.add(finger4phalanx2);
        group.add(finger4phalanx1);


        scene.add(group);


    },
    log: util.logger('hand'),
    err: util.logger('hand', true)
}


$(document).ready(main.main);