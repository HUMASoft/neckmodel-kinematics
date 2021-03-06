#include <iostream>
#include "Cia402device.h"
#include "CiA301CommPort.h"
#include "SocketCanPort.h"
#include "mainlib.h"
#include "math.h"
#include "ToolsFControl.h"

#include "SerialArduino.h"

#include "fcontrol.h"


int main ()
{
        //--sensors--
        SerialArduino tilt;
        float incSensor,oriSensor;

        double dts=0.01; //0.01
    ofstream graph("graph.csv",std::ofstream::out);

    long incl_value=20;

    //--Controllers--
    //fpi w=25 pm=70 //kept from last experiments.
    vector<double> npi ={0.2916 ,  -5.3981 ,   0.2415  ,  5.5243};
    vector<double> dpi ={0.5882 , -0.9921 , -0.5804 , 1.0000};
    SystemBlock internal1(npi,dpi);
    SystemBlock internal3(npi,dpi);
    SystemBlock internal2(npi,dpi);

//    string method("pi");
//    PIDBlock internal1(1,10,0,dts);
//    PIDBlock internal2(1,10,0,dts);
//    PIDBlock internal3(1,10,0,dts);

//    string method("2isomw10p100");
//    vector<double> npd ={0.7178 ,  -2.0893 ,  -1.3030  ,  2.9270};
//    vector<double> dpd ={0.0337 , -0.9032 , 0.0631 , 1.0000};
//    SystemBlock pd1(npd,dpd,1);
//    SystemBlock pd2(npd,dpd,1);
//    SystemBlock pd3(npd,dpd,1);

    string method("w10p60pid"); // entero
     PIDBlock external1(2,1,0.1,dts);
     PIDBlock external2(2,1,0.1,dts);
     PIDBlock external3(2,1,0.1,dts);


    //--savefiles--
    string folder("/home/humasoft/Escritorio/");
    ofstream targets (folder+method+".targets.csv");
    ofstream responses (folder+method+".responses.csv");
    ofstream controls (folder+method+".controls.csv");
//    ofstream graph("graph.csv",std::ofstream::out);


    //--Can port communications--
    SocketCanPort pm1("can1");
    CiA402Device m1 (1, &pm1);
    SocketCanPort pm2("can1");
    CiA402Device m2 (2, &pm2);
    SocketCanPort pm3("can1");
    CiA402Device m3 (3, &pm3);


    //--Neck Kinematics--
    TableKinematics a("../neck-control/arco107.csv");
    vector<double> lengths(3);


    //vars
    double ep1,ev1,cs1;
    double ep2,ev2,cs2;
    double ep3,ev3,cs3;
    double posan1, posan2, posan3;

//    Motor setup
    m1.Reset();
    m1.SwitchOn();

    m2.Reset();
    m2.SwitchOn();

    m3.Reset();
    m3.SwitchOn();


//    m1.SetupPositionMode(360,360);
//    m2.SetupPositionMode(360,360);
//    m3.SetupPositionMode(360,360);


    m3.Setup_Torque_Mode();
    m1.Setup_Torque_Mode();
    m2.Setup_Torque_Mode();

    //sysid
//    OnlineSystemIdentification id;

//    long stepsize=0;
//
//    long smallstep=5; //update orient=orient+1 every 10 steps
    long orient=1;
    long incli=1;
    float lg0=0.109;
    sleep(4);


    ToolsFControl tools;
    tools.SetSamplingTime(dts);

    //-----------Soft Start

    for (int i=1; i<incl_value; i++){
    incli ++;
    orient=1;

    tilt.readSensor(incSensor,oriSensor);

    for (double t=0;t<0.05;t+=dts){


    cout  << "incli " << incli << ",  orient " << orient  << endl;
    cout << "incli_sen: " << incSensor << " , orient_sen: " << oriSensor <<  endl;
    a.GetIK(incli,orient,lengths);
    posan1=(lg0-lengths[0])*180/(0.01*M_PI);
    posan2=(lg0-lengths[1])*180/(0.01*M_PI);
    posan3=(lg0-lengths[2])*180/(0.01*M_PI);
    cout << "TARGET: , " << posan1  << " , " << posan2 << " , " << posan3 << endl;

//    double sats=40;
//    pd1.SetSaturation(-sats,sats);
//    pd2.SetSaturation(-sats,sats);
//    pd3.SetSaturation(-sats,sats);

//        m1.SetPosition(posan1);
//        m2.SetPosition(posan2);
//        m3.SetPosition(posan3);
    //MAIN CONTROL LOOP
//       double interval=0.1*stepsize; //in seconds
//        for (double t=0;t<interval; t+=dts)
//        {


        ep1=posan1-m1.GetPosition();
        cs1=ep1 > external1;
        ev1= cs1-m1.GetVelocity();
        m1.SetTorque((ev1 > internal1));


//            probe =(ev1 > internal1);
//            cout << "probe " << probe << endl;
//            m1.SetTorque(probe);
//            id.UpdateSystem(pd1.GetState(),m1.GetPosition());


        ep2=posan2-m2.GetPosition();
        cs2=ep2 > external2;
        ev2= cs2-m2.GetVelocity();
        m2.SetTorque((ev2 > internal2));

        ep3=posan3-m3.GetPosition();
        cs3=ep3 > external3;
        ev3= cs3-m3.GetVelocity();
        m3.SetTorque((ev3 > internal3));


        cout << "ACTUAL: , " << m1.GetPosition() << " , " << m2.GetPosition() <<  " , " << m3.GetPosition() <<endl<<endl;
        graph << t << " , " << posan1 << " , " << m1.GetPosition() << " , " << posan2 << " , " << m2.GetPosition()  << " , " << posan3 <<  " , " << m3.GetPosition()  << " , " << incli << " , " << incSensor   << " , " << orient   << " , " << oriSensor <<endl;

        tools.WaitSamplingTime();
    }
 }

    for (int i=1; i<3; i++){

        //------ Program Main
    for (int i=1; i<359; i++)
    {
        incli=incl_value;
        //orient=180;
        orient += 1;
        tilt.readSensor(incSensor,oriSensor);
        for (double t=0;t<0.02;t+=dts)
        {
        //cout  << " Tiempo: " << t << endl;
//        cout  << "incli " << incli << ",  orient " << orient  << endl;
//        cout << "incli_sen: " << incSensor << " , orient_sen: " << oriSensor <<  endl;

        a.GetIK(incli,orient,lengths);
        posan1=(lg0-lengths[0])*180/(0.01*M_PI);
        posan2=(lg0-lengths[1])*180/(0.01*M_PI);
        posan3=(lg0-lengths[2])*180/(0.01*M_PI);
        //cout << "TARGET: , " << posan1  << " , " << posan2 << " , " << posan3 << endl;

    //    double sats=40;
    //    pd1.SetSaturation(-sats,sats);
    //    pd2.SetSaturation(-sats,sats);
    //    pd3.SetSaturation(-sats,sats);

//        m1.SetPosition(posan1);
//        m2.SetPosition(posan2);
//        m3.SetPosition(posan3);
        //MAIN CONTROL LOOP
//       double interval=0.1*stepsize; //in seconds
//        for (double t=0;t<interval; t+=dts)
//        {


            ep1=posan1-m1.GetPosition();
            cs1=ep1 > external1;
            ev1= cs1-m1.GetVelocity();
            m1.SetTorque((ev1 > internal1));


//            probe =(ev1 > internal1);
//            cout << "probe " << probe << endl;
//            m1.SetTorque(probe);
//            id.UpdateSystem(pd1.GetState(),m1.GetPosition());


            ep2=posan2-m2.GetPosition();
            cs2=ep2 > external2;
            ev2= cs2-m2.GetVelocity();
            m2.SetTorque((ev2 > internal2));

            ep3=posan3-m3.GetPosition();
            cs3=ep3 > external3;
            ev3= cs3-m3.GetVelocity();
            m3.SetTorque((ev3 > internal3));

//              cout << t << " , " << m1.GetPosition() << " , " << m2.GetPosition() <<  " , " << m3.GetPosition() <<endl;
//            controls << t << " , " << cs1 << " , " << cs2 <<  " , " << cs3 <<endl;
//            responses << t << " , " << m1.GetPosition() << " , " << m2.GetPosition() <<  " , " << m3.GetPosition() <<endl;

//            //            cout << t << " , " << m1.GetVelocity() << " , " << m2.GetVelocity() <<  " , " << m3.GetVelocity() <<endl;
//            //            responses << t << " , " << m1.GetVelocity() << " , " << m2.GetVelocity() <<  " , " << m3.GetVelocity() <<endl;

//            cout << t << " , " << posan1  << " , " << posan2 << " , " << posan3 << endl;
//            targets << t << " , " << posan1  << " , " << posan2 << " , " << posan3 << endl;

//            usleep(dts*1000*1000);

//        }

            cout << "error: , ep2:" << ep2 << " ,cs2 :" << cs2 << " ,ev2 :" << ev2 << endl;
            cout << "error: , ep1:" << ep1 << " ,cs1 :" << cs1 << " ,ev1 :" << ev1 << endl;
            cout << "error: , ep3:" << ep3 << " ,cs3 :" << cs3 << " ,ev3 :" << ev3 << endl;


//            cout << "ACTUAL: , " << m1.GetPosition() << " , " << m2.GetPosition() <<  " , " << m3.GetPosition() <<endl<<endl;
            graph << t << " , " << posan1 << " , " << m1.GetPosition() << " , " << posan2 << " , " << m2.GetPosition()  << " , " << posan3 <<  " , " << m3.GetPosition()  << " , " << incli << " , " << incSensor   << " , " << orient   << " , " << oriSensor <<endl;

            tools.WaitSamplingTime();
//                        usleep(dts*1000*1000);
            }


    }
    orient=1;
   }


    //-----------Soft Final
    orient=1;
    for (int i=1; i<incl_value; i++){
    incli --;


    tilt.readSensor(incSensor,oriSensor);

    for (double t=0;t<0.05;t+=dts){


    cout  << "incli " << incli << ",  orient " << orient  << endl;
    cout << "incli_sen: " << incSensor << " , orient_sen: " << oriSensor <<  endl;
    a.GetIK(incli,orient,lengths);
    posan1=(lg0-lengths[0])*180/(0.01*M_PI);
    posan2=(lg0-lengths[1])*180/(0.01*M_PI);
    posan3=(lg0-lengths[2])*180/(0.01*M_PI);
    cout << "TARGET: , " << posan1  << " , " << posan2 << " , " << posan3 << endl;

//    double sats=40;
//    pd1.SetSaturation(-sats,sats);
//    pd2.SetSaturation(-sats,sats);
//    pd3.SetSaturation(-sats,sats);

//        m1.SetPosition(posan1);
//        m2.SetPosition(posan2);
//        m3.SetPosition(posan3);
    //MAIN CONTROL LOOP
//       double interval=0.1*stepsize; //in seconds
//        for (double t=0;t<interval; t+=dts)
//        {


        ep1=posan1-m1.GetPosition();
        cs1=ep1 > external1;
        ev1= cs1-m1.GetVelocity();
        m1.SetTorque((ev1 > internal1));


//            probe =(ev1 > internal1);
//            cout << "probe " << probe << endl;
//            m1.SetTorque(probe);
//            id.UpdateSystem(pd1.GetState(),m1.GetPosition());


        ep2=posan2-m2.GetPosition();
        cs2=ep2 > external2;
        ev2= cs2-m2.GetVelocity();
        m2.SetTorque((ev2 > internal2));

        ep3=posan3-m3.GetPosition();
        cs3=ep3 > external3;
        ev3= cs3-m3.GetVelocity();
        m3.SetTorque((ev3 > internal3));


        cout << "ACTUAL: , " << m1.GetPosition() << " , " << m2.GetPosition() <<  " , " << m3.GetPosition() <<endl<<endl;
        graph << t << " , " << posan1 << " , " << m1.GetPosition() << " , " << posan2 << " , " << m2.GetPosition()  << " , " << posan3 <<  " , " << m3.GetPosition()  << " , " << incli << " , " << incSensor   << " , " << orient   << " , " << oriSensor <<endl;

        tools.WaitSamplingTime();
    }

    }

    cout << "FIN" << endl;
    m1.SetTorque(0);
    m2.SetTorque(0);
    m3.SetTorque(0);

    sleep(1);

    targets.close();
    controls.close();
    responses.close();
    return 0;
}
