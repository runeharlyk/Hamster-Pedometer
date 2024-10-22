import type { LayoutLoad } from './$types';

export const prerender = false;
export const ssr = false;

export const load = (async ({ fetch }) => {
	const result = await fetch('/api/v1/features');
	const item = await result.json();
	return {
		features: item,
		title: 'Hammie tracker',
		github: 'runeharlyk/hamster-pedometer',
		copyright: '2024 runeharlyk',
		appName: 'Hamster Tracker'
	};
}) satisfies LayoutLoad;
