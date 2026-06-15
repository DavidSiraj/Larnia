import 'package:flutter/material.dart';
import '../services/api.dart';
import '../widgets/figma.dart';
import 'teachers_screen.dart';

class ProfilesScreen extends StatefulWidget { const ProfilesScreen({super.key}); @override State<ProfilesScreen> createState() => _ProfilesScreenState(); }
class _ProfilesScreenState extends State<ProfilesScreen> {
  final api = const LarniaApi(); List<dynamic> profiles = [];
  @override void initState(){ super.initState(); load(); }
  Future<void> load() async { profiles = await api.list('/api/profiles'); if(mounted) setState((){}); }
  Future<void> createProfile() async {
    final name = TextEditingController(); final age = TextEditingController();
    final ok = await showDialog<bool>(context: context, builder: (_) => AlertDialog(backgroundColor: Colors.white, shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(24)), title: Row(children: const [LarniaMark(size: 42), SizedBox(width: 24), Text('Nový profil', style: TextStyle(color: blue, fontWeight: FontWeight.w600))]), content: SizedBox(width:420, child: Column(mainAxisSize: MainAxisSize.min, crossAxisAlignment: CrossAxisAlignment.start, children: [const Text('Meno', style: TextStyle(color: blue, fontSize: 20, fontWeight: FontWeight.w600)), TextField(controller: name, decoration: const InputDecoration(hintText: 'napr. Mária')), const SizedBox(height:24), const Text('Vek', style: TextStyle(color: blue, fontSize: 20, fontWeight: FontWeight.w600)), TextField(controller: age, keyboardType: TextInputType.number, decoration: const InputDecoration(hintText: 'napr. 13'))])), actions: [FigmaButton(text:'Zrušiť', color: blue2, width:136, onPressed: ()=>Navigator.pop(context,false)), FigmaButton(text:'+ Vytvoriť profil', width:220, onPressed: ()=>Navigator.pop(context,true))]));
    if(ok==true && name.text.trim().isNotEmpty){ await api.post('/api/profiles', {'name':name.text.trim(), 'age': int.tryParse(age.text)??0}); await load(); }
  }
  @override Widget build(BuildContext context) => Scaffold(backgroundColor: Colors.white, body: Padding(padding: const EdgeInsets.fromLTRB(45,80,45,80), child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
    const LarniaHeader(title:'Moje profily', subtitle:'Spravujte existujúce profily alebo pridajte nového člena.'), const SizedBox(height:17), Row(children:[FigmaButton(text:'Odstrániť', color: blue2, width:162), const SizedBox(width:17), FigmaButton(text:'+ Vytvoriť profil', width:222, onPressed:createProfile)]), const Spacer(), SizedBox(height:216, child: profiles.isEmpty ? Center(child: TextButton(onPressed:createProfile, child: const Text('+ Vytvoriť profil', style: TextStyle(color: blue, fontSize: 24)))) : ListView.separated(scrollDirection: Axis.horizontal, itemCount: profiles.length, separatorBuilder:(_,__)=>const SizedBox(width:23), itemBuilder:(_,i)=>FigmaCard(width:216,height:216,onTap:()=>Navigator.push(context, MaterialPageRoute(builder:(_)=>const TeachersScreen())), child: Column(mainAxisAlignment: MainAxisAlignment.center, children:[const LarniaMark(size:100), const SizedBox(height:10), Text(profiles[i]['name'], textAlign: TextAlign.center, style: const TextStyle(color: blue, fontSize:24, fontWeight: FontWeight.w600))]))), const SizedBox(height:35)])));
}
