export interface GPSCoords {
  lat: number;
  lng: number;
}

export interface RouteStep {
  distance: number;
  duration: number;
  instruction: string;
  name: string;
}

export interface RouteResult {
  totalDistance: number;
  totalDuration: number;
  steps: RouteStep[];
}

export interface POIResult {
  name: string;
  lat: number;
  lng: number;
  displayName: string;
}

const AMAP_KEY = 'b0fc38e26bb08d58b4abb2f69737e835';

export async function getCurrentAddress(lat: number, lng: number): Promise<string> {
  const url = `https://restapi.amap.com/v3/geocode/regeo?location=${lng},${lat}&key=${AMAP_KEY}`;
  const res = await fetch(url);
  const data = await res.json();
  
  if (data.status === '1' && data.regeocode) {
    return data.regeocode.formatted_address || 'Unknown location';
  } else {
    console.error('AMAP reverse geocode error:', data.info);
    return 'Unknown location';
  }
}

export async function searchNearby(lat: number, lng: number, query: string): Promise<POIResult[]> {
  const url = `https://restapi.amap.com/v3/geocode/geo?address=${encodeURIComponent(query)}&key=${AMAP_KEY}`;
  const res = await fetch(url);
  const data = await res.json();
  
  if (data.status === '1' && data.geocodes && data.geocodes.length > 0) {
    return data.geocodes.map((item: any) => ({
      name: item.formatted_address?.split('省')?.[0] || item.address || 'Unknown',
      lat: parseFloat(item.location.split(',')[1]),
      lng: parseFloat(item.location.split(',')[0]),
      displayName: item.formatted_address || item.address || 'Unknown',
    }));
  } else {
    console.error('AMAP geocode error:', data.info);
    return [];
  }
}

export async function getRoute(from: GPSCoords, to: GPSCoords): Promise<RouteResult> {
  const url = `https://restapi.amap.com/v3/direction/walking?origin=${from.lat},${from.lng}&destination=${to.lat},${to.lng}&key=${AMAP_KEY}`;
  const res = await fetch(url);
  const data = await res.json();

  if (data.status !== '1' || !data.route || !data.route.paths || data.route.paths.length === 0) {
    console.error('AMAP route error:', data.info);
    return { totalDistance: 0, totalDuration: 0, steps: [] };
  }

  const path = data.route.paths[0];
  const steps: RouteStep[] = [];
  
  if (path.steps) {
    path.steps.forEach((step: any) => {
      steps.push({
        distance: step.distance || 0,
        duration: step.duration || 0,
        instruction: step.instruction || 'straight',
        name: step.road || '',
      });
    });
  }

  return {
    totalDistance: path.distance || 0,
    totalDuration: path.duration || 0,
    steps,
  };
}

export function formatRouteSteps(route: RouteResult): string {
  if (route.steps.length === 0) return '未找到路线';

  const maneuverMap: Record<string, string> = {
    'turn': '转弯',
    'new name': '进入',
    'depart': '出发',
    'arrive': '到达目的地',
    'continue': '继续直行',
    'roundabout': '进入环岛',
    'rotary': '进入环岛',
    'end of road': '在路尽头',
    'fork': '在岔路口',
    'merge': '并道',
    'off ramp': '下匝道',
    'on ramp': '上匝道',
    'slight left': '稍向左转',
    'slight right': '稍向右转',
    'sharp left': '急左转',
    'sharp right': '急右转',
    'uturn': '调头',
    '直行': '直行',
    '左转': '左转',
    '右转': '右转',
    '向左前方': '向左前方',
    '向右前方': '向右前方',
    '向左后方': '向左后方',
    '向右后方': '向右后方',
    '到达': '到达',
  };

  let text = '';
  const maxSteps = Math.min(route.steps.length, 5);
  for (let i = 0; i < maxSteps; i++) {
    const step = route.steps[i];
    const dist = step.distance < 1000
      ? `${Math.round(step.distance)}米`
      : `${(step.distance / 1000).toFixed(1)}公里`;

    const action = maneuverMap[step.instruction] || step.instruction;
    const road = step.name ? ` ${step.name}` : '';
    text += `${action}${road}，${dist}后`;

    if (i < maxSteps - 1) {
      text += '；';
    }
  }

  const totalDist = route.totalDistance < 1000
    ? `${Math.round(route.totalDistance)}米`
    : `${(route.totalDistance / 1000).toFixed(1)}公里`;
  text += `。全程约${totalDist}，步行约${Math.round(route.totalDuration / 60)}分钟。`;  文本 += `。全程约${totalDist}，步行约${Math.round(route.totalDuration / 60)}分钟。`;
  
  return text;  返回 文本;
}

export async function navigateTo(导出 异步 函数 导航到(
  current: GPSCoords,  当前：GPSCoords，
  destination: string  目标: 字符串
): Promise<{ route: RouteResult; poi: POIResult; instructions: string } | null> {): Promise<{ route: RouteResult; poi: POIResult; instructions: string } | null> {): Promise<{ route: RouteResult; poi: POIResult; instructions: string } | null> {): Promise<{ route: RouteResult; poi: POIResult; instructions: string } | null> {
  const pois = await searchNearby(current.lat, current.lng, destination);
  if (pois.length === 0) return null;  如果 (pois.长度 === 0) 返回 空;

  const target = pois[0];  const 目标 = pois[0];
  const route = await getRoute(current, { lat: target.lat, lng: target.lng });
  const instructions = formatRouteSteps(route);

  return { route, poi: target, instructions };
}
